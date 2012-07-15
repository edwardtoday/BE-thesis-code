/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Create
 * 
 ************************************************************************/

#include "ISlice.h"
#include "IMacroblockCodec.h"
#include "ICAVLC.h"
#include "Codec/IFrame/IPredict.h"
#include "Codec/Consts4Standard.h"
#include "Common.h"
#include "Debug.h"
#include "Arch/Arch.h"
#include "Codec/ConfigureSlave.h"
#include "Codec/CAVLC.h"
#include "Codec/vlcPred.h"
#include "Codec/LoopFilter.h"
#include "Codec/CabacReader.h"
#include "Cabac.h"

void require_data_decode_I(CodingPictInfo *cpi, macroblockdata *list, predictInfo *plist, int32_t cur_r, int32_t cur_c)
{
	int32_t i = 0, j;
	int lumaStride = cpi->Size_Width,
	chromaStride = cpi->Size_Width / 2,
	refMBt = 8 * 2,
	refStride = cpi->Size_Width / 16 * refMBt;
	uint8_t *luma = cpi->lumaStart + lumaStride * cur_r * 16 + cur_c * 16,
	*chromaU = cpi->chromaUStart + lumaStride * cur_r * 4 + cur_c * 8,
	*chromaV = cpi->chromaVStart + lumaStride * cur_r * 4 + cur_c * 8;
	if ((plist[i].hasLeft = (cur_c>0)))
	{
		uint8_t *luma_p = luma - 1,
			*chromaU_p = chromaU - 1,
			*chromaV_p = chromaV - 1;
		for (j = 0; j < 16; ++j)
		{
			GetAndWait(&plist[i].left[j], luma_p, 1);
			luma_p += lumaStride;
		}
		for (j = 0; j < 8; ++j)
		{
			GetAndWait(&plist[i].data_u[8+j], chromaU_p, 1);
			GetAndWait(&plist[i].data_v[8+j], chromaV_p, 1);
			chromaU_p += chromaStride;
			chromaV_p += chromaStride;
		}
	}
	// Top
	if ((plist[i].hasTop = (cur_r>0)))
	{
		uint8_t *luma_p = luma - lumaStride,
			*chromaU_p = chromaU - chromaStride,
			*chromaV_p = chromaV - chromaStride;
		GetAndWait(plist[i].top, luma_p, 24);
		if ((cur_c+1)*16==cpi->Size_Width)
			memset(plist[i].top+16, plist[i].top[15], 8);
		GetAndWait(plist[i].data_u, chromaU_p, 8);
		GetAndWait(plist[i].data_v, chromaV_p, 8);
	}
	// TopLeft
	if (plist[i].hasLeft && plist[i].hasTop)
	{
		GetAndWait(&plist[i].topleft, luma - lumaStride - 1, 1);
		GetAndWait(&plist[i].data_u[16], chromaU - chromaStride - 1, 1);
		GetAndWait(&plist[i].data_v[16], chromaV - chromaStride - 1, 1);
	}
	else if (plist[i].hasTop)
	{
		plist[i].topleft = plist[i].top[0];
		plist[i].data_u[16] = plist[i].data_u[0];
		plist[i].data_v[16] = plist[i].data_v[0];
	}
	else if (plist[i].hasLeft)
	{
		plist[i].topleft = plist[i].left[0];
		plist[i].data_u[16] = plist[i].data_u[8];
		plist[i].data_v[16] = plist[i].data_v[8];
	}
}

void save_data(CodingPictInfo *cpi, macroblockdata *dlist, predictInfo *plist, int32_t cur_r, int32_t cur_c)
{
	//int32_t i = 0, j;
	int lumaStride = cpi->Size_Width,
		chromaStride = cpi->Size_Width / 2,
		refMBt = sizeof(refVector),
		refStride = cpi->Size_Width * 2 * refMBt;
	uint8_t *luma = cpi->lumaStart + lumaStride * cur_r * 16 + cur_c * 16,
		*chromaU = cpi->chromaUStart + lumaStride * cur_r * 4 + cur_c * 8,
		*chromaV = cpi->chromaVStart + lumaStride * cur_r * 4 + cur_c * 8,
		*ref = cpi->refVec + refStride * cur_r + cur_c * 32 * refMBt;
	for (int j = 0; j < 16; ++j)
	{
		PutAndWait(dlist[0].y[j], luma, sizeof(dlist[0].y[j]));
		luma += lumaStride;
	}
	DEBUGINFP(4, ("chroma\n"));
	for (int j = 0; j < 8; ++j)
	{
		PutAndWait(dlist[0].uv[0][j], chromaU, sizeof(dlist[0].uv[0][j]));
		PutAndWait(dlist[0].uv[1][j], chromaV, sizeof(dlist[0].uv[1][j]));
		chromaU += chromaStride;
		chromaV += chromaStride;
	}
	refVector *refs = (refVector*)ref;
	for (int i = 0; i < 32; ++i)
	{
		refs[i].refno = -1;
		refs[i].x = 0x2000;
		refs[i].y = 0x2000;
	}
}

void process_data_decode_I(CodingPictInfo *cpi, macroblockdata *dlist, predictInfo *plist, macroblockInfo *mlist)
{
	// TODO: Àî
	DEBUGBLK(9, ("process_data_decode"));

	macroblockdata pred0;

	int32_t i = 0;
	memset(&pred0, 128, sizeof(pred0));
	i = 0;
	if (mlist[i].mode_i16x16 == Intra_16x16_DC)
	{
		if (plist[i].hasLeft && plist[i].hasTop) ;	else
		if (plist[i].hasLeft)	mlist[i].mode_i16x16 = Intra_16x16_DCLEFT;	else
		if (plist[i].hasTop)	mlist[i].mode_i16x16 = Intra_16x16_DCTOP;	else
			mlist[i].mode_i16x16 = Intra_16x16_DC128;
	}
	if (mlist[i].mode_uv == Intra_8x8_DC)
	{
		if (plist[i].hasLeft && plist[i].hasTop) ;	else
		if (plist[i].hasLeft)	mlist[i].mode_uv = Intra_8x8_DCLEFT;	else
		if (plist[i].hasTop)	mlist[i].mode_uv = Intra_8x8_DCTOP;	else
			mlist[i].mode_uv = Intra_8x8_DC128;
	}
	macroblockIntraDecode(&dlist[i], cpi->baseQP + mlist->qp_delta, QPLuma2Chroma[cpi->baseQP + mlist->qp_delta], &mlist[i], &plist[i]);

}

int32_t is_data_required(int32_t idx)
{
	return 1;
}

int32_t is_data_saved(int32_t idx)
{
	return 1;
}


typedef struct tag_macroblock_info
{
	CodingPictInfo *cpi;
	macroblockInfo *mblist;
	int row;
	int allow;
	struct tag_macroblock_info *next;
} macroblock_info;

int macroblock_processor_I(void *param)
{
	macroblock_info *mi = (macroblock_info *)param;
	macroblockdata datalist;
	predictInfo predlist;
	int row = mi->row;
	int MBwidth = mi->cpi->Size_Width >> 4;
	for (int i = 0; i < MBwidth; ++i)
	{
		while (mi->allow<=i)
			Sleep(2);
		require_data_decode_I(mi->cpi, &datalist, &predlist, row, i);
		process_data_decode_I(mi->cpi, &datalist, &predlist, mi->mblist+i);
		save_data(mi->cpi, &datalist, &predlist, row, i);
		mi->next->allow = i - 1;
	}
	mi->next->allow = MBwidth;
	return 0;
}

void ISliceDecode(CodingPictInfo *cpi, Bitstream *bs, CabacReader *cabac)
{
	bool isParallel = cpi->MP * MB_E > 1.0;
	int MBheight = cpi->Size_Height >> 4,
		MBwidth = cpi->Size_Width >> 4;
	macroblockInfo *mlist = new macroblockInfo[MBheight * MBwidth];
	memset(mlist, 0, sizeof(macroblockInfo) * MBheight * MBwidth);
	macroblock_info *mis = new macroblock_info[/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP + 1];
	for (int i = 0; i <= /*ConfigureSlave::PARALLEL_SIZE*/cpi->MP; ++i)
	{
		mis[i].cpi = cpi;
		mis[i].allow = MBwidth;
		mis[i].next = mis+i+1;
	}
	mis[/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP].next = mis;

	cavlc_ref_list ref_list;
	cavlc_ref *topRef = new cavlc_ref[MBwidth * 4 + 5];
	memset(topRef, 0, sizeof(cavlc_ref) * (MBwidth * 4 + 5));
	for (int i = 0; i < MBwidth * 4 + 5; ++i)
	{
		topRef[i].coeff = 0x40;
		topRef[i].coeffC = 0x40;
		topRef[i].imode = 0x40;
	}
	ref_list.left = new cavlc_ref[4];

	int cQp = cpi->baseQP;
	int iii = 0;
	for (int r = 0; r < MBheight; ++r)
	{
		memset(ref_list.left, 0, sizeof(cavlc_ref) * 4);
		for (int j = 0; j < 4; ++j)
		{
			ref_list.left[j].coeff = 0x40;
			ref_list.left[j].coeffC = 0x40;
			ref_list.left[j].imode = 0x40;
		}
		ref_list.top = topRef + 1;
		for (int c = 0; c < MBwidth; ++c)
		{
			memset(&mlist[iii], 0, sizeof(mlist[iii]));
			mlist[iii].mbtype = 2;
			mlist[iii].mbR = r;
			mlist[iii].mbC = c;
			if (cpi->pps->cabac)
			{
				macroblock_read_cabac_I_slice(cpi, cabac, &mlist[iii], &ref_list);
			}
			else
				macroblock_read_cavlc_I_slice(bs, &mlist[iii], &ref_list);
			cQp += mlist[iii].qp_delta;
			mlist[iii].qp_delta = cQp - cpi->baseQP;
			++iii;
			ref_list.top += 4;
		}
	}
	int p = 0, p1 = p + 1, p2 = p1 + 1;
	if (/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP == 1)
		p2 = 0;
	for (int i = 0; i < MBheight; ++i)
	{
		while (mis[p2].allow != MBwidth)
			Sleep(2);
		mis[p1].allow = 0;
		mis[p].row = i;
		mis[p].mblist = mlist + i * MBwidth;
		if (/*ConfigureSlave::NO_PARALLEL*/!isParallel)
			macroblock_processor_I(mis+p);
		else
			MVCCreateMasterThread(macroblock_processor_I, mis+p);
		p = p1; p1 = p2;
		if (p2==/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP)
			p2 = 0;
		else
			++p2;
	}
	while (mis[p].allow != MBwidth)
		Sleep(2);
	if (cpi->timeId==8 && cpi->viewId==0)
	{
		//printf("============Before============\n");
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 16; ++i)
				{
					for (int j = 0; j < 16; ++j)
					{
						//printf("%4d", cpi->lumaStart[(ii*16+i)*cpi->Size_Width+jj*16+j]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
		//printf("===Before===\n");
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 8; ++i)
				{
					for (int j = 0; j < 8; ++j)
					{
						//printf("%4d", cpi->chromaUStart[(ii*8+i)*cpi->Size_Width/2+jj*8+j]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
			/*
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 8; ++i)
				{
					for (int j = 0; j < 8; ++j)
					{
						printf("%4d", cpi->chromaVStart[(ii*8+i)*cpi->Size_Width/2+jj*8+j]);
					}
					printf("\n");
				}
				printf("\n");
			}
			*/
		for (int i = 0; i < MBwidth * MBheight; ++i)
		{
			FilterMB(cpi, mlist, i);
		}
		//printf("===============After==============\n");
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 16; ++i)
				{
					for (int j = 0; j < 16; ++j)
					{
						//printf("%4d", cpi->lumaStart[(ii*16+i)*cpi->Size_Width+jj*16+j]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
		//printf("===After===\n");
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 8; ++i)
				{
					for (int j = 0; j < 8; ++j)
					{
						//printf("%4d", cpi->chromaUStart[(ii*8+i)*cpi->Size_Width/2+jj*8+j]);
					}
					//printf("\n");
				}
				//printf("\n");
			}
			/*
		for (int ii = 0; ii < MBheight; ++ii)
			for (int jj = 0; jj < MBwidth; ++jj)
			{
				for (int i = 0; i < 8; ++i)
				{
					for (int j = 0; j < 8; ++j)
					{
						printf("%4d", cpi->chromaVStart[(ii*8+i)*cpi->Size_Width/2+jj*8+j]);
					}
					printf("\n");
				}
				printf("\n");
			}
			*/
		exit(0);
	}
	else
	{
		for (int i = 0; i < MBwidth * MBheight; ++i)
		{
			FilterMB(cpi, mlist, i);
		}
	}
	delete []topRef;
	delete []mis;
	delete []mlist;
}
