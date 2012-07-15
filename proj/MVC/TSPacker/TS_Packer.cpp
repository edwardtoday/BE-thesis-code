//#include "../../version_define.h"
//#include "../powersmart_common/powersmart_common.h"
//#include "../powersmart_mmfunc/powersmart_mmfunc.h"
//#include "../powersmart_interface/powersmart_interface.h"
//#include "IWriter.h"
//#include "HTTPWriter.h"
//#include "UDPWriter.h"
//#include "SteadyWriter.h"
//#include "FileWriter.h"
#include "TS_Packer.h"
#include <cstdlib>
#include <cstring>
#include <windows.h>



#define TS_STREAM_LEN		(TS_PACKET_LEN*10240)
#define TS_PATPMT_INTERVAL	400
#define TS_TYPE_INTERVAL	500

#define FLUSH_PACKS			100
#define FLUSH_TIME			500

#define AUDIO_TYPE			0xfe

//#define SAVE_FILE	1
#define HAVE_AUDIO			1
#define TS_TIME_BASE		500

#define AUDIO_REDU_PERCENT	100

#define TS_HTTPID_BASE		10000

int gFlashCount=0;

DWORD	g_dwPCRInterval = 100;
BOOL		g_bPMTMoreInfo = TRUE;

static DWORD calculateCRC(BYTE* data, DWORD dataLength);

/*void Run_TSPacker(void *param)
{
	TS_Packer *p = (TS_Packer*)param;
	//if (p) p->RunPacker();
}*/


TS_Packer::TS_Packer()
{
	//lType = PACKER_TS;
	StreamBuf = new char[TS_STREAM_LEN];
	pVbuf = new BYTE[ENCODED_VIDEO_BUFFER];
	pAbuf = new BYTE[ENCODED_AUDIO_BUFFER];
	lStreamLen = 0;
	lStreamTail = 0;

	
	lPATPMTInterval = 0;
	lTypeAInterval = 0;

	bPATCounter = 0;
	bPMTCounter = 0;
	bVideoCounter = 0;
	bAudioCounter = 0;
	bEmptyCounter = 0;
	//fTS = NULL;
	//pAudioType = NULL;
	//lAudioType = 0;

	bAudioType = 0;
	bVideoType = 0;

	bAudioStreamID=0xc0;
	bVideoStreamID=0xe0;
	
	dwLastFlush = 0;
	llPacketCount = 0;
	dwTargetBitrate = 0;
	llPCRDelta=0;
	llLastPCRIndex = -1;
	bInsertEmptyPacket = FALSE;
	lEmptyToInsert = 0;
	dwIDStep = 0;
	dwCurrentStep = 0;
	bSecondStep = false;
	bPMTMoreInfo = FALSE;

	lTailUncomp = 0;
	pAudioPESLen = NULL;

	lAudioLeft = 0;
	lAudioPack = 0;

	bAudioInfo = 0;

	bStop = TRUE;
	bRunning = FALSE;
	//hThread = INVALID_HANDLE_VALUE;

	//pHttpWriter = NULL;
	//pUDPWriter = NULL;
	//pSteadyWriter = NULL;
	//pFileWriter = NULL;

	m_dwLastWarningTick = 0;
}

TS_Packer::~TS_Packer()
{
	//log_to_file(FLOG_NORMAL, "TS_Packer::~TS_Packer");
	
	//Close();	
	//if (pAudioType) delete pAudioType;
	if (StreamBuf) delete StreamBuf;
	if (pVbuf) delete pVbuf;
	if (pAbuf) delete pAbuf;
}

long	TS_Packer::Init(DWORD dwBitrate, BOOL bInsertEmpty, BOOL bPMTMI, DWORD dwStep, DWORD dwPID_PMT, DWORD dwPID_Video, DWORD dwPID_Audio, DWORD dwProgramNo)
{
//	Close();
	bPMTMoreInfo = bPMTMI;

	bInsertEmptyPacket = bInsertEmpty;
	lEmptyToInsert = 0;
	llPacketCount = 0;

	llBaseStamp=-1;
	
	dwTargetBitrate = dwBitrate;

	if (g_dwPCRInterval==0) g_dwPCRInterval=20;

	//llPCRDelta=dwTargetBitrate*125*30/1000/188;
	llPCRDelta=dwTargetBitrate*1000/8*g_dwPCRInterval/1000/188;

	llLastPCRIndex = -1;

	dwIDStep = dwStep/2;
	dwCurrentStep = dwIDStep;
	if (dwIDStep>0) dwCurrentStep++;
	bSecondStep = false;

	dwVideoCount=0;
	dwInputCount = 0;
	dwOutputCount = 0;

	lTailUncomp = 0;
	pAudioPESLen = NULL;

	lAudioLeft = 0;
	lAudioPack = 0;

	PID_PMT = dwPID_PMT;
	PID_Video = dwPID_Video;
	PID_Audio = dwPID_Audio;
	ProgramNo = dwProgramNo;

	//m_dwLastWarningTick = GetTickCount();

	//log_to_file(FLOG_ERR, "TS_Packer bitrate=%d, insert=%d, pmtmi=%d, delta=%d, step=%d, pid=(%x,%x,%x,%d)",
		//dwBitrate, bInsertEmpty,bPMTMoreInfo,(DWORD)llPCRDelta,dwIDStep,PID_PMT,PID_Video,PID_Audio,ProgramNo);

	return 0;
}

long	TS_Packer::SetAudioInfo(BYTE bInfo, BYTE*pbInfo)
{
	//log_to_file(FLOG_NORMAL, "TSPacker::SetAudioInfo %d",bInfo);
	if (bInfo>128) return -1;
	bAudioInfo = bInfo;
	memcpy(pbAudioInfo, pbInfo, bInfo);
	return 0;
}

long	TS_Packer::SetCodecType(BYTE bVideo, BYTE bAudio, long framerate)
{
	bAudioType = bAudio;
	bVideoType = bVideo;
	CreatePATPMT(bVideo, bAudio);
	//if (bAudio==TS_AUDIO_MPEG2 || bAudio==TS_AUDIO_MP3) 
		//bAudioStreamID=0xc0;
	//else 
		//bAudioStreamID=0xbd;
	return 0;
}

long	TS_Packer::PreStop()
{
	bStop = TRUE;
	return 0;
}

long	TS_Packer::Close()
{
	bStop = TRUE;
	//FlushBuffer();
	//log_to_file(FLOG_DEBUG, "TS_Packer::Close");
	
	for (int i=0;i<50;i++)
	{
		if (!bRunning) break;
		Sleep(20);
	}

	bRunning = FALSE;

	//myMutex_lock(&csStreamBuf, INFINITE);
	
	lPATPMTInterval = 0;

	bPATCounter = 0;
	bPMTCounter = 0;
	bVideoCounter = 0;
	bAudioCounter = 0;

	lStreamLen = 0;
	lStreamTail = 0;

	dwLastFlush = 0;

	llBaseStamp = -1;
	bFirstSyncVideo = FALSE;
	//myMutex_unlock(&csStreamBuf);

	/*if (pUDPWriter)
	{
		delete pUDPWriter;
		pUDPWriter = NULL;
	}
	if (pSteadyWriter)
	{
		delete pSteadyWriter;
		pSteadyWriter = NULL;
	}
	if (pHttpWriter)
	{
		delete pHttpWriter;
		pHttpWriter = NULL;
	}
	if (pFileWriter)
	{
		delete pFileWriter;
		pFileWriter = NULL;
	}*/

	//log_to_file(FLOG_NORMAL, "TS_Packer::Close return");
	return 0;
}

BOOL	TS_Packer::NeedFlush()
{
	if (lStreamLen>TS_PACKET_LEN*FLUSH_PACKS) return TRUE;
	if (lStreamLen>0 && GetTickCount()>dwLastFlush+FLUSH_TIME) return TRUE;
	return FALSE;
}


/*long	TS_Packer::FlushBuffer()
{
//	log_to_file(FLOG_NORMAL, "CTSStream::FlushBuffer,%d, %x",lStreamLen, (DWORD)m_pTSBuffer[0]);
	dwLastFlush = GetTickCount();

//	if (!pMemCache) return -1;
	
	//myMutex_lock(&csStreamBuf, INFINITE);
	
//#if SAVE_FILE
//	log_data_to_file("capture.ts", StreamBuf, lStreamLen,true);
//#endif

	if (lStreamTail>=lStreamLen)
	{
		char *buf = StreamBuf+lStreamTail-lStreamLen;
		for (int i=0;i<MAX_OUTPUT;i++)
		{
			if (m_pTSBuffer[i]) 
				m_pTSBuffer[i]->FlushBuffer(buf, lStreamLen);
		}
		dwOutputCount+=lStreamLen;
		//pMemCache->FillPacket(,lStreamLen/TS_PACKET_LEN);
	}
	else
	{
		long lStreamHead = TS_STREAM_LEN-lStreamLen+lStreamTail;
		long lStreamNoWrapLen = lStreamLen-lStreamTail;
		char *buf = StreamBuf+lStreamHead;
		for (int i=0;i<MAX_OUTPUT;i++)
		{
			if (m_pTSBuffer[i]) 
			{
				m_pTSBuffer[i]->FlushBuffer(buf,lStreamNoWrapLen);
				m_pTSBuffer[i]->FlushBuffer(StreamBuf,lStreamTail);
			}
		}
		dwOutputCount+=lStreamNoWrapLen;
		dwOutputCount+=lStreamTail;
		//pMemCache->FillPacket(StreamBuf+lStreamHead,(lStreamLen-lStreamTail)/TS_PACKET_LEN);
		//pMemCache->FillPacket(StreamBuf,lStreamTail/TS_PACKET_LEN);
	}
	lStreamLen = 0;
//	lStreamTail = 0;

	//myMutex_unlock(&csStreamBuf);

	gFlashCount++;

	if (dwInputCount>0x7ffffff)
	{
		dwInputCount=1;
		dwOutputCount=0;
	}
	if (dwInputCount==0) dwInputCount=1;
	if ( (gFlashCount&0x1fff)==0)
	{
		//log_to_file(FLOG_NORMAL, "TS_Packer::FlushBuffer,in:out=%u:%u, %.3f",dwInputCount, dwOutputCount, (double)dwOutputCount/(double)dwInputCount);
	}
	else if ( (gFlashCount&0x1ff)==0)
	{
		//log_to_file(FLOG_DEBUG, "TS_Packer::FlushBuffer,in:out=%lld:%lld, %.3f",dwInputCount, dwOutputCount, (double)dwOutputCount/(double)dwInputCount);
	}

	return 0;
}*/

long 	TS_Packer::WriteAdaptation(BYTE*Buffer,long lAdaptationLen)
{
	Buffer[0] = lAdaptationLen-1;
	if (lAdaptationLen>1)
	{
		Buffer[1] = 0;
		memset(Buffer+2,0xff,lAdaptationLen-2);
	}
	if (dwCurrentStep>dwIDStep && lAdaptationLen>5)
	{
		dwCurrentStep -= dwIDStep;
		DWORD index = dwCurrentStep-1;
		if (bSecondStep) index+=dwIDStep;
		bSecondStep = !bSecondStep;
		Buffer[2] = 0x80|((index>>8)&0x0f);
		Buffer[3] = (index&0xff);
		Buffer[4] = 0xff;
		Buffer[5] = 0xff;
	}
	return lAdaptationLen;
}

long 	TS_Packer::WriteAdaptation_PrivateID(BYTE*Buffer)
{
	DWORD index = dwCurrentStep-1;
	if (bSecondStep) index+=dwIDStep;
        bSecondStep = !bSecondStep;
	Buffer[0] = 0x05;
	Buffer[1] = 0x00;
	Buffer[2] = 0x80|((index>>8)&0x0f);
        Buffer[3] = (index&0xff);
	Buffer[4] = 0xff;
	Buffer[5] = 0xff;//default-id 0xffff
	
	return 6;
}

//long PCR_count=0;
long 	TS_Packer::WriteAdaptation_PCR(BYTE*Buffer,LONGLONG ts)
{
	//len
	//Buffer[0] = 0x07;
	BYTE len = 0x07;
	//flags
	Buffer[1] = 0x10; 
	//timestamp
	
	//LONGLONG ptse = ((ts*27000) % 300);
	//LONGLONG pts = ts*90;
	LONGLONG ptse = ts%300;
	LONGLONG pts = ts/300;
	
	Buffer[2] = 0xff&(pts>>25);
	Buffer[3] = 0xff&(pts>>17);
	Buffer[4] = 0xff&(pts>>9);
	Buffer[5] = 0xff&(pts>>1);

	Buffer[6] = 0x7e|((ptse>>8)&0x01)|((pts<<7)&0x80);
	Buffer[7] = ptse&0xff;

	if (dwCurrentStep>dwIDStep)
	{
		dwCurrentStep -= dwIDStep;
		DWORD index = dwCurrentStep-1;
		if (bSecondStep) index+=dwIDStep;
        bSecondStep = !bSecondStep;
		Buffer[8] = 0x80|((index>>8)&0x0f);
        Buffer[9] = (index&0xff);
		Buffer[10] = 0xff;
		Buffer[11] = 0xff;
		len = 0x11;
	}

//	PCR_count++;

//	if ( (PCR_count&0xFF) == 0)
//	{
//		log_to_file(FLOG_NORMAL, "TS_Packer, WriteAdaptation_PCR, %lld, %x %x %x %x %x %x",ts,
//								Buffer[2],Buffer[3],Buffer[4],Buffer[5],Buffer[6],Buffer[7]);
//	}

//	{
//		BYTE *pos = Buffer;
//		unsigned int d1,d2,d3;
//		d1=(unsigned int)pos[2]*256*256*256+
//			(unsigned int)pos[3]*256*256+
//			(unsigned int)pos[4]*256+
//			(unsigned int)pos[5];
//		d2=(pos[6]&0x80)>>7;
//		d3=(pos[6]&0x01)*256+pos[7];
//		
//		LONGLONG pcr=((__int64)d1*2+(__int64)d2)*300+(__int64)d3;  
//		pcr/=(27*1000);
//
//		log_to_file(FLOG_DEBUG, "%I64d == %I64d", ts, pcr);
//	}
	Buffer[0] = len;
	return (len+1);
}

long WritePESHeader(BYTE StreamID,BYTE*Buffer,LONGLONG ts, USHORT len, LONGLONG dts)
{
	//start
	Buffer[0] = 0x00;
	Buffer[1] = 0x00;
	Buffer[2] = 0x01;
	Buffer[3] = StreamID;
	//len=0
	Buffer[4] = (len>>8)&0xff;
	Buffer[5] = len&0xff;
	//flags
	Buffer[6] = 0x84;

	//pts
	LONGLONG pts = ts*90/10000;
	Buffer[9] = 0x21|((pts>>29)&0x0e);
	Buffer[10] = (pts>>22)&0xff;
	Buffer[11] = 0x01|((pts>>14)&0xfe);
	Buffer[12] = (pts>>7)&0xff;
	Buffer[13] = 0x01|((pts<<1)&0xfe);
	
	if (dts<=0) 
	{
		Buffer[7] = 0x80;
		Buffer[8] = 0x05;
		return 14;
	}
	else
	{
		Buffer[7] = 0xc0;
		Buffer[8] = 0x0a;

		Buffer[9] |= 0x31;

		LONGLONG ddts = dts*90/10000;
		Buffer[14] = 0x11|((ddts>>29)&0x0e);
		Buffer[15] = (ddts>>22)&0xff;
		Buffer[16] = 0x01|((ddts>>14)&0xfe);
		Buffer[17] = (ddts>>7)&0xff;
		Buffer[18] = 0x01|((ddts<<1)&0xfe);
		return 19;
	}
/*
	{
		BYTE*pos = (BYTE*)Buffer+9;
		unsigned int d1,d2,d3;
		d1=(unsigned int)(pos[0]&0x0E)/2;
		d2=(unsigned int)(pos[1]*256+pos[2])/2;
		d3=(unsigned int)(pos[3]*256+pos[4])/2;
					
		LONGLONG pcr=((__int64)d1<<30)+((__int64)d2<<15)+(__int64)d3;
		pcr /= 90;
		log_to_file(FLOG_DEBUG, "%I64d == %I64d", ts, pcr);
	}
*/
}

long WriteAUDelimiter(BYTE*Buffer, BOOL bSync)
{
	Buffer[0] = 0x00;
	Buffer[1] = 0x00;
	Buffer[2] = 0x00;
	Buffer[3] = 0x01;
	Buffer[4] = 0x09;

	Buffer[5] = bSync?0x50:0x30;
	return 6;
}

long	TS_Packer::InsertVideoFrame(BYTE *Buffer,long lBufLen,LONGLONG pts,LONGLONG dts)
{
	
//	log_data_to_file("video.dat", &lBufLen, sizeof(long), true);
//	log_data_to_file("video.dat", &ts, sizeof(LONGLONG), true);
//	log_data_to_file("video.dat", Buffer, lBufLen, true);

	dwVideoCount++;

	if ((dwVideoCount&0xff)==0)
	{
		//log_to_file(FLOG_DEBUG, "InsertVideoFrame, %d, %lld %lld",lBufLen,pts,dts);
	}

	dwInputCount+=lBufLen;
	//if (NeedFlush()) FlushBuffer();

	if (lPATPMTInterval+TS_PATPMT_INTERVAL<dts/10000LL)
	{
		InsertPAT();
		InsertPMT();
		lPATPMTInterval = dts/10000LL;
	}
	
	if (llBaseStamp==-1) 
	{
		llBaseStamp=dts/10000LL;
		//log_to_file(FLOG_NORMAL,"InsertVideoFrame BaseStamp=%lld, %lld",llBaseStamp,dts);
	}
		
	BOOL bIFrame=FALSE;
	//long frame_type = FrameTypeH264((unsigned char*)Buffer, lBufLen);
	/*if (frame_type==H264_I)
	{
		bIFrame = TRUE;
		bFirstSyncVideo = TRUE;
		InsertPAT();
		InsertPMT();
		lPATPMTInterval = dts/10000;
		lTypeAInterval = 0;
	}*/

	LONGLONG llDTS = dts;
	/*if (frame_type==H264_B)
	{
		llDTS = 0;
	}*/

	int empty_toinsert=0;
	if (bInsertEmptyPacket && lEmptyToInsert>0)
	{
		long inserttimes = lBufLen/188;
		if (inserttimes>0)
		{
			empty_toinsert = (lEmptyToInsert+inserttimes-1)/inserttimes;
			if (empty_toinsert<0) empty_toinsert=0;
		}
	}

	BYTE TSPacket[TS_PACKET_LEN];

	BOOL bFirst=TRUE;
	long lBufOffset=0;
	long lBufOri = lBufLen;
	
	while(lBufLen>0)
	{
		memset(TSPacket,0xff,TS_PACKET_LEN);
		TSPacket[0] = 0x47;
		
		TSPacket[1] = 0x00|((PID_Video&0x1f00)>>8);//*/0x01;
		TSPacket[2] = (PID_Video&0xff);//*/0xe1;

		TSPacket[3] = 0x10;

		TSPacket[3] |= bVideoCounter;
		bVideoCounter = (bVideoCounter+1)%16;

		long lOffset=4;
		long lPESHeaderLen=0;

		if (dwIDStep>0 && dwCurrentStep>2*dwIDStep)
		{
			dwCurrentStep -= dwIDStep;
			bSecondStep = !bSecondStep;
		}

		if (bFirst)
		{
			TSPacket[1] |= 0x40;
			
			if (lBufLen>TS_PACKET_LEN-lOffset)	//there will be adaptation field
			{
				TSPacket[3] |= 0x20;	
				LONGLONG pcr = (dts-TS_TIME_BASE*10000)/10*27;
				if (dwTargetBitrate>0 && bInsertEmptyPacket) 
					pcr = llPacketCount*188*8*27000/dwTargetBitrate+llBaseStamp*27000;
				lOffset += WriteAdaptation_PCR(TSPacket+lOffset,pcr);
			}
			
			lPESHeaderLen = WritePESHeader(bVideoStreamID,TSPacket+lOffset,pts,0,llDTS);
			lOffset += lPESHeaderLen;
			bFirst = FALSE;

			//lOffset += WriteAUDelimiter(TSPacket+lOffset, bIFrame);
		}
	
		if (lBufLen<TS_PACKET_LEN-lOffset)	//PES tail ts-packet
		{
			TSPacket[3] |= 0x20;
			if (lPESHeaderLen>0)
			{
				lOffset = 4;
				WriteAdaptation(TSPacket+lOffset,TS_PACKET_LEN-lOffset-lBufLen-lPESHeaderLen);
				lOffset = TS_PACKET_LEN-lBufLen-lPESHeaderLen;
				WritePESHeader(bVideoStreamID,TSPacket+lOffset,pts,0,llDTS);
				lOffset += lPESHeaderLen;
			}
			else
			{
				WriteAdaptation(TSPacket+lOffset,TS_PACKET_LEN-lOffset-lBufLen);
				lOffset = TS_PACKET_LEN-lBufLen;
			}
		}
		else if (lPESHeaderLen==0)// && bInsertEmptyPacket)
		{
			if (bInsertEmptyPacket && llLastPCRIndex+llPCRDelta<=llPacketCount)
			{
				TSPacket[3] |= 0x20;
				LONGLONG pcr = (dts-TS_TIME_BASE*10000)*27/10;//*27000
				if (dwTargetBitrate>0) //pcr=llPacketCount*188*1000*27000/128/dwTargetBitrate;
					pcr = llPacketCount*188*8*27000/dwTargetBitrate+llBaseStamp*27000;

				lOffset += WriteAdaptation_PCR(TSPacket+lOffset,pcr);
				llLastPCRIndex = llPacketCount;
			}
			/*
			else if (!bInsertEmptyPacket && llLastPCRIndex+llPCRDelta/2<=llPacketCount)
			{
				TSPacket[3] |= 0x20;
				LONGLONG pcr = (ts-TS_TIME_BASE)*27000;
				if (dwTargetBitrate>0) //pcr=llPacketCount*188*1000*27000/128/dwTargetBitrate;
					pcr = llPacketCount*188*8*27000/dwTargetBitrate+llBaseStamp*27000;

				lOffset += WriteAdaptation_PCR(TSPacket+lOffset,pcr);
				llLastPCRIndex = llPacketCount;
			}*/
			else if (dwCurrentStep>dwIDStep)
			{
				dwCurrentStep -= dwIDStep;
				TSPacket[3] |= 0x20;
				lOffset += WriteAdaptation_PrivateID(TSPacket+lOffset);
			}	
		}

		long lCopyLen = TS_PACKET_LEN-lOffset;
		if (lBufLen<lCopyLen) lCopyLen=lBufLen;
		memcpy(TSPacket+lOffset,Buffer+lBufOffset,lCopyLen);
		lBufOffset += lCopyLen;
		lBufLen -= lCopyLen;

		InsertTS(TSPacket);
		
		int nextpcr = (long)(llLastPCRIndex+llPCRDelta-llPacketCount);
		for (int i=0;i<empty_toinsert;i++)
		{
			if (lEmptyToInsert==0 || nextpcr==0) break;
			InsertEmpty();
			lEmptyToInsert --;
			nextpcr --;
		}
	}

	//kbps
	if (bInsertEmptyPacket)
	{
		lEmptyToInsert = (dts/10000LL-llBaseStamp)*dwTargetBitrate*125/1000/188-llPacketCount;

		//log_to_file(FLOG_DEBUG, "TSPacker InsertEmpty:%d,dts=%lld, packetcount=%lld", lEmptyToInsert,dts,llPacketCount);


		DWORD tick = GetTickCount();

		LONGLONG a=llPacketCount*188*8/dwTargetBitrate;
        LONGLONG b=(dts/10000-llBaseStamp); 		

		if (a>b+200)
		{
			//log_to_file(FLOG_NORMAL, "[%x]TSPacker frame=%d, empty=%d, %lld, %lld, I=%d, gap=%u, a-b=%d", 
				//(long)this,lBufOri,lEmptyToInsert,dts, llPacketCount, bIFrame,
				//tick-m_dwLastWarningTick,(long)(a-b));

//		if (a>b+300) log warning
			m_dwLastWarningTick = tick;
		}

		if (lEmptyToInsert>0 && 
			(lEmptyToInsert*188*8/dwTargetBitrate>1000 || lStreamLen+lEmptyToInsert*TS_PACKET_LEN>TS_STREAM_LEN))
		{
			//当待插入的空包发送时间超过1000ms，或待插入的空包会直接导致Buffer上溢时
			//则不插入这些空包，而直接增加llPacketCount，在后续计算带宽时会认为那些空包已经被插入
			//log_to_file(FLOG_NORMAL, "TSPacker too many empty-packets %d",lEmptyToInsert);
			llPacketCount += lEmptyToInsert;
			lEmptyToInsert = 0;
		}
	}
	return 0;
}


/*
Buffer：从队列中取出的音频帧
lBufLen：Buffer中有效数据长度
*/
long	TS_Packer::InsertAudioFrame(char*Buffer,long lBufLen,LONGLONG ts,BOOL bSync)
{
	if (bAudioType==0) return 0;

	dwInputCount+=lBufLen;

	//if (NeedFlush()) FlushBuffer();

	//ts /=10000;
	//if (llBaseStamp==-1) llBaseStamp=ts-TS_TIME_BASE;
	//ts-=llBaseStamp;
	//if (ts<TS_TIME_BASE) ts=TS_TIME_BASE;

	BYTE TSPacket[TS_PACKET_LEN];//保存TS包
	BOOL bFirst=TRUE;
	long lBufOffset=0;
	long paklen=lBufLen;

	while(paklen>0)
	{
		memset(TSPacket,0xff,TS_PACKET_LEN);
		TSPacket[0] = 0x47;
		TSPacket[1] = 0x00+((PID_Audio&0x1f00)>>8);//0x01;
		TSPacket[2] = (PID_Audio&0xff);//0xe2;

		TSPacket[3] = 0x10;
		
		TSPacket[3] |= bAudioCounter;
		bAudioCounter = (bAudioCounter+1)%16;
		long lOffset=4;
		long lPESHeaderLen=0;
		long lPESSizePos=-1;
		long packtype=0;//zhe

		lAudioPack ++;
		
		if (bFirst)
		{
			bFirst = FALSE;
			if (lAudioLeft>0)//有剩余未打数据
			{
				if (lAudioPack==1)
				{
					TSPacket[1] |= 0x40;
					lPESHeaderLen = WritePESHeader(bAudioStreamID,TSPacket+lOffset,ts,lBufLen+lAudioLeft+8,0);
					lPESSizePos = lOffset+4;
	        lOffset += lPESHeaderLen;
				}
				else if (pAudioPESLen)
        {
                                     long size = pAudioPESLen[0]*256+pAudioPESLen[1];
                                     size += paklen;
                                     pAudioPESLen[0] = (size/256);
                                     pAudioPESLen[1] = (size%256);
        }
				
				if (lOffset+lAudioLeft+paklen>TS_PACKET_LEN)//打不完
				{
					//打入剩余
					memcpy(TSPacket+lOffset, pAudioLeft, lAudioLeft);
					lOffset += lAudioLeft;
					lAudioLeft = 0;
					//打入新帧头数据
					long len = TS_PACKET_LEN-lOffset;
					memcpy(TSPacket+lOffset, Buffer, len);
					paklen -= len;
					lBufOffset += len;

					long pos = InsertTS(TSPacket, 2);//插入未完结音频包
					if (lPESSizePos>0 && pos>=0) pAudioPESLen = (BYTE*)StreamBuf+pos+lPESSizePos;
					continue;
				}
				else
				{
					TSPacket[3] |= 0x20;
					lOffset += WriteAdaptation(TSPacket+lOffset,TS_PACKET_LEN-paklen-lAudioLeft-lOffset);
					memcpy(TSPacket+lOffset, pAudioLeft, lAudioLeft);
					lOffset += lAudioLeft;
					lAudioLeft = 0;
					memcpy(TSPacket+lOffset, Buffer, paklen);
					paklen = 0;

					InsertTS(TSPacket, 1);//插入完结音频包

					lAudioPack = 0;
					pAudioPESLen = NULL;
					break;
				}
			}
			else//新开始打一帧
			{
				TSPacket[1] |= 0x40;
				lPESHeaderLen = WritePESHeader(bAudioStreamID,TSPacket+lOffset,ts,lBufLen+8,0);
				lPESSizePos = lOffset+4;
				lOffset += lPESHeaderLen;
				packtype = 2;
			}
		}
		
		if (paklen<TS_PACKET_LEN-lOffset)//结尾包
		{
			long adaptlen = TS_PACKET_LEN-lOffset-paklen;
			if (adaptlen<=AUDIO_REDU_PERCENT*lAudioPack*TS_PACKET_LEN/100)
			{	//冗余较少，可以完结				
				TSPacket[3] |= 0x20;
				if (lPESHeaderLen>0)
				{
					lOffset = 4;
					WriteAdaptation(TSPacket+lOffset,TS_PACKET_LEN-lOffset-paklen-lPESHeaderLen);
					lOffset = TS_PACKET_LEN-paklen-lPESHeaderLen;
					lPESHeaderLen = WritePESHeader(bAudioStreamID,TSPacket+lOffset,ts,lBufLen+8,0);
					lPESSizePos = lOffset+4;
					lOffset += lPESHeaderLen;
				}
				else
				{
					WriteAdaptation(TSPacket+lOffset,TS_PACKET_LEN-lOffset-paklen);
					lOffset = TS_PACKET_LEN-paklen;
				}

				lAudioPack = 0;
				pAudioPESLen=NULL;
				packtype = 1;
			}
			else//剩余字节记入pAudioLeft，不再打入
			{
				memcpy(pAudioLeft, Buffer+lBufOffset, paklen);
				lAudioLeft = paklen;
				bAudioCounter = (bAudioCounter+15)%16;
				lAudioPack --;
				break;
			}
		}
		
		long lCopyLen = TS_PACKET_LEN-lOffset;
		if (paklen<lCopyLen) lCopyLen=paklen;
		memcpy(TSPacket+lOffset,Buffer+lBufOffset,lCopyLen);
		lOffset += lCopyLen;
		lBufOffset += lCopyLen;
		paklen -= lCopyLen;
		
		long pos = InsertTS(TSPacket, packtype);
		if (lPESSizePos>0 && pos>=0)
			pAudioPESLen = (BYTE*)StreamBuf+pos+lPESSizePos;
	}
	return 0;
}

long	TS_Packer::InsertTS(BYTE *TSPacket, long packtype)//将打好的TS包放在缓冲区
{
	if (lStreamLen+lTailUncomp+TS_PACKET_LEN>TS_STREAM_LEN) 
	{
		//log_to_file(FLOG_ERR, "TSPacker drop packets buffer full %d %d %d %d",lStreamLen,lTailUncomp,TS_PACKET_LEN,TS_STREAM_LEN);
		//丢弃当前所有已打好的TS Buffer
		lTailUncomp = 0;
		lStreamLen = 0;
		lAudioLeft = 0;
		lAudioPack = 0;
		return -1;
	}
	//::EnterCriticalSection(&csStreamBuf);

	llPacketCount ++;	//记录缓冲区中TS包的数目
	if (dwIDStep>0) dwCurrentStep++;//?

	//myMutex_lock(&csStreamBuf, INFINITE);//锁住缓冲区

	long pos = (lStreamTail+lTailUncomp)%TS_STREAM_LEN;
	memcpy(StreamBuf+pos,TSPacket,TS_PACKET_LEN);
	
	if (packtype==1)	//complete audio
	{
		lStreamTail = (lStreamTail+lTailUncomp+TS_PACKET_LEN)%TS_STREAM_LEN;
		lStreamLen += (lTailUncomp+TS_PACKET_LEN);
		lTailUncomp = 0;
	}
	else if (packtype==2) //uncomplete audio
	{
		lTailUncomp += TS_PACKET_LEN;
	}
	else
	{
		if (lTailUncomp>0) lTailUncomp+=TS_PACKET_LEN;
		else
		{
			lStreamTail = (lStreamTail+TS_PACKET_LEN)%TS_STREAM_LEN;
			lStreamLen += TS_PACKET_LEN;
		}
	}
	//::LeaveCriticalSection(&csStreamBuf);
	//myMutex_unlock(&csStreamBuf);

	return pos;
}

long	TS_Packer::InsertEmpty()//向缓冲区中插入空的TS包
{
	EMPTYPACK[3] &= 0xf0;//空包的pid为保留pid0x1fff，为何这里通过标志位来设定？
	EMPTYPACK[3] |= bEmptyCounter;
	bEmptyCounter = (bEmptyCounter+1)%16;
	return InsertTS(EMPTYPACK);
}

long	TS_Packer::InsertPAT()//向缓冲区中插入pat包
{
	//log_to_file(FLOG_NORMAL, "TS_Packer, InsertPAT");
	PAT[3] &= 0xF0;
	PAT[3] |= bPATCounter;
	bPATCounter = (bPATCounter+1)%16;
	return InsertTS(PAT);
}

long	TS_Packer::InsertPMT()
{
	//log_to_file(FLOG_NORMAL, "TS_Packer, InsertPMT");
	PMT[3] &= 0xF0;
	PMT[3] |= bPMTCounter;
	bPMTCounter = (bPMTCounter+1)%16;
	return InsertTS(PMT);
}

long	TS_Packer::CreatePATPMT(BYTE bVideo, BYTE bAudio)
{
	memset(PAT,0xff,TS_PACKET_LEN);
	PAT[0] = 0x47;
	PAT[1] = 0x60;
	PAT[2] = 0x00;
	PAT[3] = 0x10;//TS head
	
	//pat table
	PAT[4] = 0x00;//point field
	PAT[5] = 0x00;//table id
	PAT[6] = 0xb0;
	PAT[7] = 0x0d;//section length, 从此字段下一个开始，到crc最后一个字节结束
	PAT[8] = 0x00;
	PAT[9] = 0x00;
	PAT[10] = 0xc1;//version number==0
	PAT[11] = 0x00;//section number==0
	PAT[12] = 0x00;//last section number==0
	PAT[13] = 0x00;
	PAT[14] = 0x01;//program number == 1
	PAT[15] = 0xe1;
	PAT[16] = 0xe0;//pmt_pid
	DWORD crc = calculateCRC(PAT+5, 12);
	PAT[17] = crc>>24;
	PAT[18] = (crc>>16) & 0xff;
	PAT[19] = (crc>>8) & 0xff;
	PAT[20] = crc & 0xff;

	memset(PMT, 0xff, TS_PACKET_LEN);
	PMT[0] = 0x47;
	//PMT[1] = 0x61;
	PMT[1] = 0x40+((PID_PMT&0x1f00)>>8);//0x41;
	PMT[2] = (PID_PMT&0xff);    //0xe0;
	PMT[3] = 0x10;

	PMT[4] = 0x00;
	PMT[5] = 0x02;
	PMT[6] = 0xb0;
/*	
	if (bAudio)
        {
		if (bPMTMoreInfo) 
			PMT[7] = 0x37;
		else PMT[7] = 0x17;
        }
        else
        {
                if (bPMTMoreInfo) PMT[7] = 0x32;
		else PMT[7] = 0x12;
        }
*/	
	PMT[8] = ((ProgramNo&0xff00)>>8) ;//0x00;
	PMT[9] = ProgramNo&0xff; //0x01;

	PMT[10] = 0xc1;
	PMT[11] = 0x00;
	PMT[12] = 0x00;
	PMT[13] = 0xe0|((PID_Video&0x1f00)>>8);//0xe1;
	PMT[14] = (PID_Video&0xff); //0xe1;
	PMT[15] = 0xf0;
	
	int offset = 16;
	
	if (bPMTMoreInfo)
	{
		PMT[offset++]=0x0c;
		PMT[offset++]=0x05;
		PMT[offset++]=0x04;
		PMT[offset++]=0x48;
		PMT[offset++]=0x44;
		PMT[offset++]=0x4d;
		PMT[offset++]=0x56;
		PMT[offset++]=0x88;
		PMT[offset++]=0x04;
		PMT[offset++]=0x0f;
		PMT[offset++]=0xff;
		PMT[offset++]=0xfc;
		PMT[offset++]=0xfc;
	}
	else
	{
		PMT[offset++]=0x00;
	}
	//offset = 29
	PMT[offset++]=bVideo;
	PMT[offset++]=0xe0|((PID_Video&0x1f00)>>8); //0xe1;
	PMT[offset++]=(PID_Video&0xff);				//0xe1;

	PMT[offset++]=0xf0;

	if (bPMTMoreInfo)
	{
		PMT[offset++]=0x14;
		PMT[offset++]=0x05;
		PMT[offset++]=0x08;
		PMT[offset++]=0x48;
		PMT[offset++]=0x44;
		PMT[offset++]=0x4d;
		PMT[offset++]=0x56;
		PMT[offset++]=0xff;
		PMT[offset++]=0x1b;
		PMT[offset++]=0x26; 
		PMT[offset++]=0x2f;
		PMT[offset++]=0x28;
		PMT[offset++]=0x04;
		PMT[offset++]=0x64;
		PMT[offset++]=0x00;
		PMT[offset++]=0x28;
		PMT[offset++]=0x3f;
		PMT[offset++]=0x2a;
		PMT[offset++]=0x02;
		PMT[offset++]=0x7e;
		PMT[offset++]=0xff;
	}
	else
	{
		PMT[offset++]=0x00;
	}	

	if (bAudio)
	{
		if (bAudioInfo>0)
		{
			memcpy(PMT+offset, pbAudioInfo, bAudioInfo);
			PMT[offset+1] = (PMT[offset+1]&0xe0)|((PID_Audio&0x1f00)>>8);
			PMT[offset+2] = (PID_Audio&0xff);
			offset += bAudioInfo;
		}	
		else
		{
			PMT[offset++] = bAudio;
			PMT[offset++] = 0xe0+((PID_Audio&0x1f00)>>8);	//0xe1;
			PMT[offset++] = (PID_Audio&0xff);				//0xe2;
			PMT[offset++] = 0xf0;
			PMT[offset++] = 0x00;
		}
	}

	PMT[7] = offset-4;

	DWORD pmtcrc = calculateCRC(PMT+5, offset-5);
	PMT[offset++] = pmtcrc>>24;
	PMT[offset++] = (pmtcrc>>16) & 0xff;
	PMT[offset++] = (pmtcrc>>8) & 0xff;
	PMT[offset++] = pmtcrc & 0xff;

	memset(EMPTYPACK,0xff,TS_PACKET_LEN);
	EMPTYPACK[0] = 0x47;
	EMPTYPACK[1] = 0x1f;
	EMPTYPACK[2] = 0xff;//空包的pid为0x1fff
	EMPTYPACK[3] = 0x00;//空包的adaptation field为01

	return 0;
}



static DWORD calculateCRC(BYTE* data, DWORD dataLength) {
	DWORD crc = 0xFFFFFFFF;
		
	return crc;
}