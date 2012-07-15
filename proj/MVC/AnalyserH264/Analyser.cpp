#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <vector>
#include "Codec/Bitstream.h"
#include "Analyser.h"
#include "Codec/SequenceParam.h"
#include "Codec/PictureParam.h"
#include "Codec/SliceParam.h"
#include "Codec/Consts4Standard.h"

using namespace std;

#pragma  warning(disable:4018)

#undef ASSERT
#define ASSERT(b) { if (!(b)) { printf("Assert Error (%d)!!!!\n", __LINE__); exit(0); } }

const char *inputFileName = "3d-432x240-reorder-nocabac-gop64.264";//"testH264.264";

char buf[400000000];
int size;
int tnal;
char *nalunits[1000000];
int nalsize[1000000];
typedef struct tagPictRefInfo {
	bool ShortRef;
	int FrameNum;
	int FrameNumWrap;
	int PicNum;
	bool LongRef;
	int LongTermPicNum;
	int MaxLongTermFrameIdx;
} PictRefInfo;

PictRefInfo picts[10000][8];
SliceParameters psp[10000][8];

#pragma warning(disable:4996)

Bitstream bs;
SequenceParametersSet sps;
PictureParametersSet pps;
SliceParameters sp;

void init()
{
	FILE *f = fopen(inputFileName, "rb");	//rb+ 读写打开一个二进制文件，只允许读写数据。
	size = fread(buf, 1, sizeof(buf), f);	//按字节读入buf,size为成功读取的元素个数
	fclose(f);
}


void Comment(char *str)
{
	printf("%20s  ", str);
}

int ReadCode(uint32_t len)//n位直接读取
{
	int code = eg_read_direct(&bs, len);
	printf("Code %08lX, %d\n", code, len);
	return code;
}

int ReadFlag()//1位直接读取
{
	int code = eg_read_direct1(&bs);
	printf("Flag %d\n", code);
	return code;
}

uint32_t ReadUvlc()//ue(v)
{
	uint32_t uvlc = eg_read_ue(&bs);
	printf("Uvlc %d\n", uvlc);
	return uvlc;
}

int32_t ReadSvlc()//se(v)
{
	int32_t svlc = eg_read_se(&bs);
	printf("Svlc %d\n", svlc);
	return svlc;
}

void ByteAlign()//byte_aligned( ) 的规定如下：
				//— 如果比特流的当前位置是在字节边界，即，比特流中的下一比特是字节第一个比特，byte_aligned( )的
				//返回值为TRUE。
				//— 否则，byte_aligned( )的返回值为FALSE。
{
	BitstreamByteAlign(&bs);
}

void ResetBitstream()
{
	BitstreamReset(&bs);
}


int getNalType(int id)
{
	Bitstream bs;
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	eg_read_direct(&bs, 1);//forbidden_zero_bit
	eg_read_direct(&bs, 2);//nal_ref_idc
	return eg_read_direct(&bs, 5);//nal_unit_type
}

int getNalRefIdc(int id)
{
	Bitstream bs;
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	eg_read_direct(&bs, 1);
	return eg_read_direct(&bs, 2);
}

void readSequenceParameterSet(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	ASSERT(eg_read_direct(&bs, 1)==0);
	eg_read_direct(&bs, 2);
	eg_read_direct(&bs, 5);
	Comment("SPS: profile_idc");
	int profileidc = ReadCode(8);
	//ASSERT(ReadCode(8) == HIGH_PROFILE);//profile_idc=100
	Comment("SPS: constrained_set0_flag");
	ReadFlag();
	//ASSERT(ReadFlag() == 0);//constraint_set0_flag=0
	Comment("SPS: constrained_set1_flag");
	ReadFlag();
	//ASSERT(ReadFlag() == 0);//constraint_set1_flag=0
	Comment("SPS: constrained_set2_flag");
	ReadFlag();
	//ASSERT(ReadFlag() == 0);//constraint_set2_flag=0
	Comment("SPS: constrained_set3_flag");
	ReadFlag();
	//ASSERT(ReadFlag() == 0);//constraint_set3_flag=0
	Comment("SPS: reserved_zero_4bits");
	ASSERT(ReadCode(4) == 0);//reserved_zero_4bits=0
	Comment("SPS: level_idc");
	ReadCode(8);//level_idc=21
	Comment("SPS: seq_parameter_set_id");
	printf("\tParameter Set Id = %d\n", ReadUvlc());//seq_parameter_set_id
	
	Comment("--- fidelity range extension syntax ---");
	Comment("ReadFrext");
	Comment("SPS: chroma_format_idc");
	ASSERT(ReadUvlc() == 1); // 4:2:0 chroma_format_idc=1
	//chroma_format_idc 是指如6.2 节所提出的，与亮度取样对应的色度取样。chroma_format_idc 的值应该在0
	//到3的范围内（包括0和3）。当chroma_format_idc不存在时，应推断其值为1（4：2：0的色度格式）。
	Comment("SPS: bit_depth_luma_minus8");
	ASSERT(ReadUvlc() == 0); // BitDepthY = 8 + bit_depth_luma_minus8,  QpBdOffsetY = 6 * bit_depth_luma_minus8 bit_depth_luma_minus8=0
	Comment("SPS: bit_depth_chroma_minus8");
	ASSERT(ReadUvlc() == 0); // BitDepthC = 8 + bit_depth_chroma_minus8,  QpBdOffsetC = 6 * ( bit_depth_chroma_minus8 + residual_colour_transform_flag) bit_depth_chroma_minus8=0
	Comment("SPS: qpprime_y_zero_transform_bypass_flag");
	ASSERT(ReadFlag() == 0);//qpprime_y_zero_transform_bypass_flag = 0
	Comment("SPS: seq_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);//seq_scaling_matrix_present_flag=0

	Comment("SPS: log2_max_frame_num_minus4");
	int log2nFramesM4 = ReadUvlc();//int log2nFrameM4=log2_max_frame_num_minus4
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0; // getPicOrderCntType()
	ASSERT(ReadUvlc() == picOrderCntType);//pic_order_cnt_type=0
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	int log2nPocM4 = ReadUvlc();// log2_max_pic_order_cnt_lsb_minus4=5
	sps.log2nPoc = log2nPocM4 + 4;

	Comment("SPS: num_ref_frames");
	sps.numRefFrames = ReadUvlc();//num_ref_frames
	printf("\tNum Ref Frames = %d\n", sps.numRefFrames);
	Comment("SPS: gaps_in_frame_num_value_allowed_flag");
	ReadFlag();//gaps_in_frame_num_value_allowed_flag=1

	Comment("SPS: pic_width_in_mbs_minus_1");
	sps.width  = (ReadUvlc() + 1) << 4;//pic_width_in_mbs_minus1
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	sps.height = (ReadUvlc() + 1) << 4;//pic_height_in_map_units_minus1
	printf("\tSize = (%d,%d)\n", sps.width, sps.height);
	Comment("SPS: frame_mbs_only_flag");
	ASSERT(ReadFlag() == 1);//frame_mbs_only_flag=1
	Comment("SPS: direct_8x8_inference_flag");
	ASSERT(ReadFlag() == 1);//direct_8x8_inference_flag=1
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);//frame_cropping_flag=0
	Comment("SPS: vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);//vui_parameters_present_flag=0
}


void readSubsetSequenceParameterSet(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_SUBSET_SPS);

	Comment("SPS: profile_idc");
	int profidc = ReadCode(8);
	printf("\tProfile IDC = %d\n", profidc);
	Comment("SPS: constrained_set0_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: constrained_set1_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: constrained_set2_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: constrained_set3_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: constrained_set4_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: reserved_zero_3bits");
	ASSERT(ReadCode(3) == 0);
	Comment("SPS: level_idc");
	ASSERT(ReadCode(8) == 21);
	Comment("SPS: seq_parameter_set_id");
	ReadUvlc();

	Comment("--- fidelity range extension syntax ---");
	Comment("ReadFrext");
	Comment("SPS: chroma_format_idc");
	ASSERT(ReadUvlc() == 1); // 4:2:0
	Comment("SPS: bit_depth_luma_minus8");
	ASSERT(ReadUvlc() == 0); // BitDepthY = 8 + bit_depth_luma_minus8,  QpBdOffsetY = 6 * bit_depth_luma_minus8
	Comment("SPS: bit_depth_chroma_minus8");
	ASSERT(ReadUvlc() == 0); // BitDepthC = 8 + bit_depth_chroma_minus8,  QpBdOffsetC = 6 * ( bit_depth_chroma_minus8 + residual_colour_transform_flag)
	Comment("SPS: qpprime_y_zero_transform_bypass_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: seq_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);

	int log2nFramesM4 = 5;
	ASSERT(ReadUvlc() == log2nFramesM4);

	Comment("SPS: pic_order_cnt_type");
	int picOrderCntType = 0;
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	ASSERT(ReadUvlc() == 5); // ?

	Comment("SPS: num_ref_frames");
	ReadUvlc();
	Comment("SPS: required_frame_num_update_behaviour_flag");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: pic_width_in_mbs_minus_1");
	ASSERT(ReadUvlc() == (sps.width>>4)-1);
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	ASSERT(ReadUvlc() == (sps.height>>4) - 1);
	Comment("SPS: frame_mbs_only_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: direct_8x8_inference_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);

	Comment("SUBSET SPS: bit_equal_to_one");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: num_views_minus_1");
	sps.viewCount = ReadUvlc() + 1;
	printf("\tView Count = %d\n", sps.viewCount);

	sps.viewCodingOrder.clear();
	for (int i = 0; i < sps.viewCount; ++i)
	{
		Comment("SPS: view_id[i]");
		sps.viewCodingOrder.push_back(ReadUvlc());
	}
	printf("\tView Order = ");
	for (int i = 0; i < sps.viewCodingOrder.size(); ++i)
		printf("%d ", sps.viewCodingOrder[i]);
	printf("\n");

	sps.anchorRefsList0.clear();
	ArrayList<int> tmp;
	for (int i = 0; i < sps.viewCount; ++i)
	{
		sps.anchorRefsList0.push_back(tmp);
	}
	sps.anchorRefsList1 = sps.anchorRefsList0;
	sps.nonAnchorRefsList0 = sps.anchorRefsList0;
	sps.nonAnchorRefsList1 = sps.anchorRefsList0;
	for (int i = 1; i < sps.viewCount; ++i)
	{
		int coding = sps.viewCodingOrder[i];
		Comment("SPS: num_anchor_refs_l0[i]");
		int nAnchorRefs = ReadUvlc();
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: anchor_ref_l0[i][j]");
			int refv = ReadUvlc();
			sps.anchorRefsList0[coding].push_back(refv);
		}

		Comment("SPS: num_anchor_refs_l1[i]");
		nAnchorRefs = ReadUvlc();
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: anchor_ref_l1[i][j]");
			int refv = ReadUvlc();
			sps.anchorRefsList1[coding].push_back(refv);
		}
	}

	for (int i = 1; i < sps.viewCount; ++i)
	{
		int coding = sps.viewCodingOrder[i];
		Comment("SPS: num_non_anchor_refs_l0[i]");
		int nAnchorRefs = ReadUvlc();
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: non_anchor_ref_l0[i][j]");
			int refv = ReadUvlc();
			sps.nonAnchorRefsList0[coding].push_back(refv);
		}

		Comment("SPS: num_non_anchor_refs_l1[i]");
		nAnchorRefs = ReadUvlc();
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: non_anchor_ref_l1[i][j]");
			int refv = ReadUvlc();
			sps.nonAnchorRefsList1[coding].push_back(refv);
		}
	}
	printf("\tAnchor:");
	for (int i = 0; i < sps.viewCount; ++i)
	{
		printf("\t\tView %d: ", i);
		for (int j = 0; j < sps.anchorRefsList0[i].size(); ++j)
			printf("%d ", sps.anchorRefsList0[i][j]);
		printf("    ****    ");
		for (int j = 0; j < sps.anchorRefsList1[i].size(); ++j)
			printf("%d ", sps.anchorRefsList1[i][j]);
		printf("\n");
	}
	printf("\tNonAnchor:");
	for (int i = 0; i < sps.viewCount; ++i)
	{
		printf("\t\tView %d: ", i);
		for (int j = 0; j < sps.nonAnchorRefsList0[i].size(); ++j)
			printf("%d ", sps.nonAnchorRefsList0[i][j]);
		printf("    ****    ");
		for (int j = 0; j < sps.nonAnchorRefsList1[i].size(); ++j)
			printf("%d ", sps.nonAnchorRefsList1[i][j]);
		printf("\n");
	}

	Comment("SPS: num_level_values_signalled");
	ASSERT(ReadUvlc() == 0);

	Comment("SUBSET SPS: mvc_vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SUBSET SPS: Additional_extension2_flag");
	ASSERT(ReadFlag() == 0);
}

void readPictureParameterSet(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_PPS);

	Comment("PPS: pic_parameter_set_id");
	ReadUvlc();//pic_parameter_set_id
	Comment("PPS: seq_parameter_set_id");
	ReadUvlc();//seq_parameter_set_id
	Comment("PPS: entropy_coding_mode_flag");
	ASSERT(ReadFlag() == 0); // CAVLC, 1 CABAC entropy_coding_mode_flag=0
	Comment("PPS: pic_order_present_flag");
	ReadFlag();//pic_order_present_flag=1
	Comment("PPS: num_slice_groups_minus1");
	ASSERT(ReadUvlc() == 0);//num_slice_groups_minus1=0

	Comment("PPS: num_ref_idx_l0_active_minus1");
	pps.num_ref_idx_l0_active = ReadUvlc() + 1;//num_ref_idx_l0_active_minus1
	Comment("PPS: num_ref_idx_l1_active_minus1");
	pps.num_ref_idx_l1_active = ReadUvlc() + 1;//num_ref_idx_l1_active_minus1
	Comment("PPS: weighted_pred_flag");
	ReadFlag();//weighted_pred_flag=0
	Comment("PPS: weighted_bipred_idc");
	ReadCode(2);//weighted_bipred_idc=0
	Comment("PPS: pic_init_qp_minus26");
	pps.baseQP = ReadSvlc() + 26;//pic_init_qp_minus26
	printf("\tBase QP = %d\n", pps.baseQP);
	Comment("PPS: pic_init_qs_minus26");
	ASSERT(ReadSvlc() == 0);//pic_init_qs_minus26
	Comment("PPS: chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);//chroma_qp_index_offset=0
	Comment("PPS: deblocking_filter_parameters_present_flag");
	ASSERT(ReadFlag() == 0);//deblocking_filter_control_present_flag=0
	Comment("PPS: constrained_intra_pred_flag");
	ASSERT(ReadFlag() == 0);//constrained_intra_pred_flag=0
	Comment("PPS: redundant_pic_cnt_present_flag");
	ASSERT(ReadFlag() == 0);//redundant_pic_cnt_present_flag=0

	//more_rbsp_data()
	Comment("PPS: transform_8x8_mode_flag");
	ASSERT(ReadFlag() == 1);//transform_8x8_mode_flag=1
	Comment("PPS: pic_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);//pic_scaling_matrix_present_flag=0
	Comment("PPS: second_chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);//second_chroma_qp_index_offset=0
}

void readSlicePrefix(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ReadCode(2);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_CODED_SLICE_PREFIX);
	Comment("NALU HEADER: svc_mvc_flag");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: non_idr_flag");
	sp.IDR = (ReadFlag() == 0);
//	printf("\tIDR = %d\n", sp.IDR);
	Comment("NALU HEADER: priority_id");
	sp.priority_id = ReadCode(6);
//	printf("\tPriority ID = %d\n", sp.priority_id);
	Comment("NALU HEADER: view_id");
	sp.viewId = ReadCode(10);
//	printf("\tView ID = %d\n", sp.viewId);
	Comment("NALU HEADER: temporal_id");
	int temporal_id = ReadCode(3); // ?
//	printf("\tTemporal ID = %d\n", temporal_id);
	Comment("NALU HEADER: anchor_pic_flag");
	sp.isAnchor = (ReadCode(1) == 1);
//	printf("\tAnchor PIC = %d\n", sp.isAnchor);
	Comment("NALU HEADER: inter_view_flag");
	int inter_view = ReadCode(1); // ?
//	printf("\tInter View = %d\n", inter_view);
	Comment("NALU HEADER: reserved_zero_one_bit");
	ASSERT(ReadCode(1) == 0);
}

void readSliceHeader(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	int long_term_reference_flag = 0;
	int adaptive_ref_pic_marking_mode_flag = 0;
	int ref_pic_list_reordering_flag0 = 0;
	int ref_pic_list_reordering_flag1 = 0;
	int num_ref_idx_l0_active = pps.num_ref_idx_l0_active;
	int num_ref_idx_l1_active = pps.num_ref_idx_l1_active;
	vector<int> mc;
	vector<int> mc1;
	vector<int> mc2;
	vector<pair<int, int> > reo0, reo1;
	int nal_ref_idc = ReadCode(2);
	Comment("NALU HEADER: nal_unit_type");
	int nal_unit_type = ReadCode(5);
	//if (nal_unit_type == 20)
	//{
	//	Comment("NALU HEADER: svc_mvc_flag");
	//	ASSERT(ReadFlag() == 0);
	//	Comment("NALU HEADER: non_idr_flag");
	//	sp.IDR = (ReadFlag()==0);
	//	Comment("NALU HEADER: priority_id");
	//	sp.priority_id = ReadCode(6);
	//	Comment("NALU HEADER: view_id");
	//	sp.viewId = ReadCode(10);
	//	Comment("NALU HEADER: temporal_id");
	//	int temporal_id = ReadCode(3);
	//	printf("\t\tTemporal Level = %d\n", temporal_id);
	//	Comment("NALU HEADER: anchor_pic_flag");
	//	sp.isAnchor = (ReadCode(1) == 1);
	//	Comment("NALU HEADER: inter_view_flag");
	//	int inter_view_flag = ReadCode(1);
	//	printf("\t\tInter view flag = %d\n", inter_view_flag);
	//	Comment("NALU HEADER: reserved_zero_one_bit");
	//	ASSERT(ReadCode(1) == 0);
	//}
	Comment("SH: first_mb_in_slice");
	ASSERT(ReadUvlc() == 0);//first_mb_in_slice=0
	Comment("SH: slice_type");
	sp.SliceType = ReadUvlc();//slice_type
	if (sp.SliceType >= 5 )
		sp.SliceType -= 5;
	Comment("SH: pic_parameter_set_id");
	int ppsid = ReadUvlc();//pic_parameter_set_id
	ASSERT(ppsid == (nal_unit_type==20));//ppsid == false?
	Comment("SH: frame_num");
	sp.frameNum = ReadCode(sps.log2nFrames);//frame_num
	if( nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )//ual_unit+type==5
	{
		Comment("SH: idr_pic_id");
		int idrPicId = ReadUvlc();//idr_pic_id
		printf("\tIDR Picture ID = %d\n", idrPicId);
	}
	Comment("SH: pic_order_cnt_lsb");
	// Time Id
	sp.timeId = ReadCode(sps.log2nPoc);//pic_order_cnt_lsb
	//Comment("SH: delta_pic_order_cnt_bottom");
	//int delta_pic_order_cnt_bottom = ReadSvlc();//delta_pic_order_cnt_bottom
	//printf("\tdelta_pic_order_cnt_bottom = %d\n", delta_pic_order_cnt_bottom);

	if (sp.SliceType==1 || sp.SliceType==6) // B:1,6
	{
		Comment("SH: direct_spatial_mv_pred_flag");
		int direct_spatial_mv_pred_flag = ReadFlag();//direct_spatial_mv_pred_flag
		//ASSERT(direct_spatial_mv_pred_flag == 1)
	}

	if (sp.SliceType==0 || sp.SliceType==1 || sp.SliceType==5 || sp.SliceType==6 || sp.SliceType==3 || sp.SliceType==8 )//P:0,5  SP:3,8  B:1,6
	{
		Comment("SH: num_ref_idx_active_override_flag");
		int num_ref_idx_active_override_flag = ReadFlag();//num_ref_idx_active_override_flag
		if (num_ref_idx_active_override_flag)
		{
			Comment("SH: num_ref_idx_l0_active_minus1");
			num_ref_idx_l0_active = ReadUvlc() + 1;//num_ref_idx_l0_active_minus1
//			printf("\tNumber REF Index L0 = %d\n", num_ref_idx_l0_active);
			if (sp.SliceType==1 || sp.SliceType==6)//B
			{
				Comment("SH: num_ref_idx_l1_active_minus1");
				num_ref_idx_l1_active = ReadUvlc() + 1;//num_ref_idx_l1_active_minus1
//				printf("\tNumber REF Index L1 = %d\n", num_ref_idx_l1_active);
			}
		}

		//ref_pic_list_reordering()
		Comment("RIR: ref_pic_list_reordering_flag");
		//if( slice_type != I && slice_type != SI )
		ref_pic_list_reordering_flag0 = ReadFlag();//ref_pic_list_reordering_flag_l0
//		if (ref_pic_list_reordering_flag0)
//			printf("\tREF Picture List 0 Reorder\n");
		if (ref_pic_list_reordering_flag0)
		{
			int reordering_of_pic_nums_idc;
			int tt = 10;
			do {
				reordering_of_pic_nums_idc = ReadUvlc();//reordering_of_pic_nums_idc
				int va = 0;
				if (reordering_of_pic_nums_idc != 3)
					va = ReadUvlc();//abs_diff_pic_num_minus1(0,1),long_term_pic_num(2)
				reo0.push_back(make_pair(reordering_of_pic_nums_idc, va+1));
 				printf("\t\treordering_of_pic_nums_idc = %d(%d)\n", reordering_of_pic_nums_idc, va);
// 				if (reordering_of_pic_nums_idc==0 || reordering_of_pic_nums_idc==1)
// 				{
// 					printf("\t\tabs_diff_pic_num = %d\n", va + 1);
// 				}
// 				else if (reordering_of_pic_nums_idc == 2)
// 				{
// 					ASSERT(false);
// 				}
// 				else if (reordering_of_pic_nums_idc == 4)
// 				{
// 					printf("\t\tabs_diff_view_idx = %d\n", va + 1);
// 				}
// 				else if (reordering_of_pic_nums_idc == 5)
// 				{
// 					printf("\t\tabs_diff_view_idx = %d\n", va + 1);
// 				}
				tt--;//ASSERT(tt--);
			} while (reordering_of_pic_nums_idc != 3);
		}
		if (sp.SliceType==1 || sp.SliceType==6) // B
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			ref_pic_list_reordering_flag1 = ReadFlag();//ref_pic_list_reordering_flag_l1
// 			if (ref_pic_list_reordering_flag1)
// 				printf("\tREF Picture List 1 Reorder\n");
			int tt = 3;
			if (ref_pic_list_reordering_flag1)
			{
				int reordering_of_pic_nums_idc;
				do {
					reordering_of_pic_nums_idc = ReadUvlc();//reordering_of_pic_nums_idc
					int va = 0;
					if (reordering_of_pic_nums_idc != 3)
						va = ReadUvlc();////abs_diff_pic_num_minus1(0,1),long_term_pic_num(2)
					reo1.push_back(make_pair(reordering_of_pic_nums_idc, va+1));
 					printf("\t\treordering_of_pic_nums_idc = %d (%d)\n", reordering_of_pic_nums_idc, va);
// 					if (reordering_of_pic_nums_idc==0 || reordering_of_pic_nums_idc==1)
// 					{
// 						printf("\t\tabs_diff_pic_num = %d\n", va + 1);
// 					}
// 					else if (reordering_of_pic_nums_idc == 2)
// 					{
// 						ASSERT(false);
// 					}
// 					else if (reordering_of_pic_nums_idc == 4)
// 					{
// 						printf("\t\tabs_diff_view_idx = %d\n", va + 1);
// 					}
// 					else if (reordering_of_pic_nums_idc == 5)
// 					{
// 						printf("\t\tabs_diff_view_idx = %d\n", va + 1);
// 					}
					tt--;//ASSERT(tt--);
				} while (reordering_of_pic_nums_idc != 3);
			}
		}
	}

	if (nal_ref_idc)
	{
		if (nal_unit_type==5)
		{
			Comment("DRPM: no_output_of_prior_pics_flag");
			int no_output_of_prior_pics_flag = ReadFlag();//no_output_of_prior_pics_flag
			printf("\t\tno_output_of_prior_pics_flag = %d\n", no_output_of_prior_pics_flag);
			Comment("DRPM: long_term_reference_flag");
			long_term_reference_flag = ReadFlag();//long_term_reference_flag
			printf("\t\tlong_term_reference_flag = %d\n", long_term_reference_flag);
		}
		else
		{
			Comment("adaptive_ref_pic_marking_mode_flag");
			adaptive_ref_pic_marking_mode_flag = ReadFlag();//adaptive_ref_pic_marking_mode_flag
			if (adaptive_ref_pic_marking_mode_flag)
			{
				int memory_management_control_operation;
				int pb, pa;
				do {
					memory_management_control_operation = ReadUvlc();//memory_management_control_operation
					pa = pb = 0;
					switch (memory_management_control_operation)
					{
					case 3://difference_of_pic_nums_minus1,long_term_frame_idx
						pb = ReadUvlc();

					case 1://difference_of_pic_nums_minus1
					case 2://long_term_pic_num
					case 4://max_long_term_frame_idx_plus1
					case 6://long_term_frame_idx
						pa = ReadUvlc();

					case 5:
					default : 
						;
					}
					mc.push_back(memory_management_control_operation);
					mc1.push_back(pa);
					mc2.push_back(pb);
//					printf("\t\tmemory_management_control_operation = %d (%d,%d)\n", memory_management_control_operation, pa, pb);

					if (memory_management_control_operation==0)
					{

					}
					else if (memory_management_control_operation==1)
					{
						int fn = sp.frameNum - pa - 1;
						bool find = false;
						for (int x = 0; x < 100; ++x)
						{
							if (picts[x][sp.viewId].ShortRef && picts[x][sp.viewId].FrameNum==fn)
							{
								picts[x][sp.viewId].ShortRef = false;
								find = true;
								break;
							}
						}
						if (!find)
						{
							//ASSERT(0);
						}
					}
					else
					{
						//ASSERT(0);
					}

				} while (memory_management_control_operation != 0);
			}
		}
	}

	Comment("SH: slice_qp_delta");
	sp.QP_Delta = ReadSvlc();//slice_qp_delta
	printf("\tFrame = (Time=%3d, View=%d) %d(%c) %s %s FrameNum=%3d QP Delta=%d\n", sp.timeId, sp.viewId, sp.SliceType, 
		"PBI0123456789012345678901234567890"[sp.SliceType], sp.isAnchor?"Anchor ":"nAnchor",
		sp.IDR?"IDR ":"nIDR", sp.frameNum, sp.QP_Delta);

	for (int i = 0; i < 100; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			if (picts[i][j].ShortRef)
			{
				picts[i][j].FrameNum = psp[i][j].frameNum;
				if (picts[i][j].FrameNum > sp.frameNum)
				{
					picts[i][j].FrameNumWrap = picts[i][j].FrameNum - sps.nFramesM1 - 1;
				}
				else
				{
					picts[i][j].FrameNumWrap = picts[i][j].FrameNum;
				}
				picts[i][j].PicNum = picts[i][j].FrameNumWrap;
			}
			if (picts[i][j].LongRef)
			{
				ASSERT(false);
			}
		}
	}
	vector<pair<int, int> > list0, list1;
	if (sp.SliceType == 2)
	{
		return; // Nothing
	}
	else if (sp.SliceType == 1)
	{
		int j = sp.viewId;
		for (int i = sp.timeId - 1; i >= 0; --i)
		{
			if (picts[i][j].ShortRef)
				list0.push_back(make_pair(i, j));
		}
		for (int i = sp.timeId; i < 100; ++i)
		{
			if (picts[i][j].ShortRef)
				list0.push_back(make_pair(i, j));
		}
		for (int i = sp.timeId + 1; i < 100; ++i)
		{
			if (picts[i][j].ShortRef)
				list1.push_back(make_pair(i, j));
		}
		for (int i = sp.timeId; i >= 0; --i)
		{
			if (picts[i][j].ShortRef)
				list1.push_back(make_pair(i, j));
		}
	}
	else if (sp.SliceType == 0)
	{
		int j = sp.viewId;
		for (int i = 0; i < 100; ++i)
		{
			if (picts[i][j].ShortRef)
				list0.push_back(make_pair(i, j));
		}
		for (int i = list0.size() - 1; i >= 0; --i)
			for (int j = 0; j < i; ++j)
			{
				if (picts[list0[j].first][list0[j].second].FrameNum <
					picts[list0[j+1].first][list0[j+1].second].FrameNum)
				{
					pair<int, int> tmp = list0[j];
					list0[j] = list0[j+1];
					list0[j+1] = tmp;
				}
			}
	}
	else
	{
		cout << "----------slice_type == " << sp.SliceType << endl;
		//ASSERT(0);
	}
	
	cout << "----------start sp.isAnchor..." << endl;
	cout << "----------sp.isAnchor = " << sp.isAnchor << endl;
	/*
	if (sp.isAnchor)
	{
		cout << "----------for1!" << endl;
		for (int i = 0; i < sps.anchorRefsList0[sp.viewId].size(); ++i)
			list0.push_back(make_pair(sp.timeId, sps.anchorRefsList0[sp.viewId][i]));
		cout << "----------for2!" << endl;
		for (int i = 0; i < sps.anchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(make_pair(sp.timeId, sps.anchorRefsList1[sp.viewId][i]));
		cout << "----------for3!" << endl;
	}
	else
	{
		cout << "----------for1!" << endl;
		for (int i = 0; i < sps.nonAnchorRefsList0[sp.viewId].size(); ++i)
			list0.push_back(make_pair(sp.timeId, sps.nonAnchorRefsList0[sp.viewId][i]));
		cout << "----------for2!" << endl;
		for (int i = 0; i < sps.nonAnchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(make_pair(sp.timeId, sps.nonAnchorRefsList1[sp.viewId][i]));
		cout << "----------for3!" << endl;
	}
	*/

#define RPLR_NEG 0
#define RPLR_POS 1
#define RPLR_LONG 2
#define RPLR_END 3
#define RPLR_VIEW_NEG 4
#define RPLR_VIEW_POS 5
	cout << "----------ref_pic_list_reordering_flag0" << endl;
	if (ref_pic_list_reordering_flag0)
	{
		UInt  uiPicNumPred    = sp.frameNum;
		UInt  uiIndex         = 0;   
		UInt  uiCommand       = 0;   
		UInt  uiIdentifier    = 0;   
		cout << "----------" << "JVT-V043" << endl;// JVT-V043 
		UInt  uiPicViewIdx    = -1;   
		for (int i = 0; i < reo0.size(); ++i)
		{   
			uiCommand = reo0[i].first;
			uiIdentifier = reo0[i].second - 1;
			if (uiCommand == 3)
				break;
			if( uiCommand == RPLR_LONG )   
			{   
				cout << "----------" << "long-term index" << endl;//===== long-term index =====   
				//ASSERT(0);
			}   
			else if (uiCommand == RPLR_NEG || uiCommand == RPLR_POS) // JVT-V043    
			{   
				cout << "----------" << "short-term index" << endl;//===== short-term index =====   
				int uiAbsDiff = uiIdentifier + 1;   
				if( uiCommand == RPLR_NEG )
					uiAbsDiff = -uiAbsDiff;
				uiPicNumPred += uiAbsDiff;   
				uiPicNumPred &= sps.nFramesM1;
				uiIdentifier = uiPicNumPred;   

				cout << "----------" << "get frame" << endl;//----- get frame -----   
				int ft = -1;
				for (int x = 0; x < 100; ++x)
					if (picts[x][sp.viewId].ShortRef && picts[x][sp.viewId].FrameNum == uiIdentifier)
					{
						ft = x;
						break;
					}
				if (ft < 0)
				{
					//ASSERT(0);
				}
				int p = -1;
				for (int x = uiIndex; x < list0.size(); ++x)
					if (list0[x].first == ft && list0[x].second == sp.viewId)
					{
						list0.erase(list0.begin() + x);
						break;
					}
				list0.insert(list0.begin() + uiIndex, make_pair(ft, sp.viewId));
				uiIndex++;   
			} // short-term   
			else // 4 or 5   
			{ 
				cout << "----------" << "4 or 5" << endl;
				UInt uiAbsDiff = uiIdentifier + 1;   
				// JVT-W066   
				UInt uiMaxRef;
				if (sp.isAnchor)
					uiMaxRef = sps.anchorRefsList0[sp.viewId].size();
				else
					uiMaxRef = sps.nonAnchorRefsList0[sp.viewId].size();

				if( uiCommand == RPLR_VIEW_NEG )   
					uiAbsDiff = uiMaxRef - uiAbsDiff;
				uiPicViewIdx +=   uiAbsDiff;   
				if( uiPicViewIdx >= uiMaxRef )   
					uiPicViewIdx -= uiMaxRef;
				// JVT-W066   
				uiIdentifier = uiPicViewIdx;    


				UInt targetViewId;
				if (sp.isAnchor)
					targetViewId = sps.anchorRefsList0[sp.viewId][uiIdentifier];
				else
					targetViewId = sps.nonAnchorRefsList0[sp.viewId][uiIdentifier];

				cout << "----------" << "find picture in reference list" << endl;//----- find picture in reference list -----   
				for (int x = 0; x < list0.size(); ++x)
					if (list0[x].first == sp.timeId && list0[x].second == targetViewId)
					{   
						list0.erase(list0.begin() + x);
						break;
					}   
				list0.insert(list0.begin() + uiIndex, make_pair(sp.timeId, targetViewId));

				uiIndex++;   
			} // inter-view    
		} // while   
	}
	cout << "----------ref_pic_list_reordering_flag1" << endl;
	if (ref_pic_list_reordering_flag1)
	{
		UInt  uiPicNumPred    = sp.frameNum;
		UInt  uiIndex         = 0;   
		UInt  uiCommand       = 0;   
		UInt  uiIdentifier    = 0;   
		// JVT-V043   
		UInt  uiPicViewIdx    = -1;   
		for (int i = 0; i < reo1.size(); ++i)
		{   
			uiCommand = reo1[i].first;
			uiIdentifier = reo1[i].second - 1;
			if (uiCommand == 3)
				break;
			if( uiCommand == RPLR_LONG )   
			{   
				//===== long-term index =====   
				ASSERT(0);
			}   
			else if (uiCommand == RPLR_NEG || uiCommand == RPLR_POS) // JVT-V043    
			{   
				//===== short-term index =====   
				int uiAbsDiff = uiIdentifier + 1;   
				if( uiCommand == RPLR_NEG )
					uiAbsDiff = -uiAbsDiff;
				uiPicNumPred += uiAbsDiff;   
				uiPicNumPred &= sps.nFramesM1;
				uiIdentifier = uiPicNumPred;   

				//----- get frame -----   
				int ft = -1;
				for (int x = 0; x < 100; ++x)
					if (picts[x][sp.viewId].ShortRef && picts[x][sp.viewId].FrameNum == uiIdentifier)
					{
						ft = x;
						break;
					}
					if (ft < 0)
					{
						ASSERT(0);
					}
					int p = -1;
					for (int x = uiIndex; x < list1.size(); ++x)
						if (list1[x].first == ft && list1[x].second == sp.viewId)
						{
							list1.erase(list1.begin() + x);
							break;
						}
						list1.insert(list1.begin() + uiIndex, make_pair(ft, sp.viewId));
						uiIndex++;   
			} // short-term   
			else // 4 or 5   
			{   
				UInt uiAbsDiff = uiIdentifier + 1;   
				// JVT-W066   
				UInt uiMaxRef;
				if (sp.isAnchor)
					uiMaxRef = sps.anchorRefsList1[sp.viewId].size();
				else
					uiMaxRef = sps.nonAnchorRefsList1[sp.viewId].size();

				if( uiCommand == RPLR_VIEW_NEG )   
					uiAbsDiff = uiMaxRef - uiAbsDiff;
				uiPicViewIdx +=   uiAbsDiff;   
				if( uiPicViewIdx >= uiMaxRef )   
					uiPicViewIdx -= uiMaxRef;
				// JVT-W066   
				uiIdentifier = uiPicViewIdx;    


				UInt targetViewId;
				if (sp.isAnchor)
					targetViewId = sps.anchorRefsList1[sp.viewId][uiIdentifier];
				else
					targetViewId = sps.nonAnchorRefsList1[sp.viewId][uiIdentifier];

				//----- find picture in reference list -----   
				for (int x = 0; x < list1.size(); ++x)
					if (list1[x].first == sp.timeId && list1[x].second == targetViewId)
					{   
						list1.erase(list1.begin() + x);
						break;
					}   
					list1.insert(list1.begin() + uiIndex, make_pair(sp.timeId, targetViewId));

					uiIndex++;   
			} // inter-view    
		} // while   
	}

// 	if (sp.SliceType == 0 || sp.SliceType == 1 || sp.SliceType == 5 || sp.SliceType == 6)
// 	{
// 		printf("List 0 : ");
// 		for (int i = 0; i < list0.size(); ++i)
// 			printf("(%d,%d),", list0[i].first, list0[i].second);
// 		printf("\n");
// 
// 		if (sp.SliceType == 1 || sp.SliceType == 6)
// 		{
// 			printf("List 1 : ");
// 			for (int i = 0; i < list1.size(); ++i)
// 				printf("(%d,%d),", list1[i].first, list1[i].second);
// 			printf("\n");
// 		}
// 
// 	}

	if (sp.SliceType == 0 || sp.SliceType == 1 || sp.SliceType == 5 || sp.SliceType == 6)
	{
		printf("\tList 0 : ");
		for (int i = 0; i < list0.size(); ++i)
			printf("(%d,%d),", list0[i].first, list0[i].second);
		printf("\n");

		if (sp.SliceType == 1 || sp.SliceType == 6)
		{
			printf("\tList 1 : ");
			for (int i = 0; i < list1.size(); ++i)
				printf("(%d,%d),", list1[i].first, list1[i].second);
			printf("\n");
		}

	}

	while (list0.size() > num_ref_idx_l0_active)
		list0.pop_back();
	while (list1.size() > num_ref_idx_l1_active)
		list1.pop_back();



	// 参考图像
	cout << "----------参考图像" << endl;
	if (nal_ref_idc)
	{
		if (sp.IDR)
		{
			int j = sp.viewId;
			for (int i = 0; i < 100; ++i)
			{
				picts[i][j].ShortRef = false;
				picts[i][j].LongRef = false;
			}
			if (long_term_reference_flag)
			{
				picts[sp.timeId][sp.viewId].LongRef = true;
				picts[sp.timeId][sp.viewId].MaxLongTermFrameIdx = 0;
				ASSERT(false);
			}
			else
			{
				picts[sp.timeId][sp.viewId].ShortRef = true;
				picts[sp.timeId][sp.viewId].MaxLongTermFrameIdx = -1;
			}
		}
		else
		{
			if (adaptive_ref_pic_marking_mode_flag)//8.2.5.4
			{
				for (int k = 0; k < mc.size(); ++k)
				{
					switch (mc[k])
					{
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
						//ASSERT(false);
					default : ;
					}
				}
			}
			else//8.2.5.3
			{
				int tref = 0;
				int j = sp.viewId;
				for (int i = 0; i < 100; ++i)
					if (picts[i][j].ShortRef || picts[i][j].LongRef)
					{
						++tref;
					}
				if (tref == sps.numRefFrames)
				{
					int mfw = 100000, t = 0, v = 0;
					for (int i = 0; i < 100; ++i)
						for (int j = 0; j < 8; ++j)
							if (picts[i][j].ShortRef)
							{
								if (picts[i][j].FrameNumWrap < mfw)
								{
									mfw = picts[i][j].FrameNumWrap;
									t = i;
									v = j;
								}
							}
							if (mfw < 100000)
							{
								picts[t][v].ShortRef = false;
							}
				}
			}
			if (!picts[sp.timeId][sp.viewId].LongRef)
			{
				picts[sp.timeId][sp.viewId].ShortRef = true;
			}
		}
	}
	else // 8.2.5.2
	{
		// TODO :
	}
}

void getnalunits()
{
	cout << size << endl;//成功读取的元素个数
	tnal = 0;
	for (int i = 0; i < size; ++i)
	{
		//在解码过程开始时，解码器把其当前的位置初始化为字节流的起始位置。然后提取，并丢弃每一个
		//leading_zero_8bits 语法元素（如果存在的话），移动当前位置至某一时刻的字节处，直到比特流的当前位置紧接
		//的四个字节为四字节序列0x00000001。
		if (buf[i]==0 && buf[i+1]==0 && buf[i+2]==0 && buf[i+3]==1)
		{
			nalunits[tnal] = buf + i + 4;//nal单元开始地址
			//当字节流里的紧接的四个字节构成四字节序列0x00000001，对比特流中下一个字节（为zero_byte语法元
			//素）进行提取并丢弃时，字节流的当前位置设为紧接被丢弃的字节的字节位置。
			//提取与丢弃比特流中下一个三字节序列（为start_code_prefix_one_3bytes），且比特流当前位置设为此紧
			//接被丢弃的3字节序列的字节的位置。
			int j = i + 4, k = j;
			while (j<size)
			{
				if (buf[j-1]==0 && buf[j-2]==0 && buf[j-3]==0 && buf[j]==3)
				{//处理0x00000003的特殊情况
				}
				else
					buf[k++] = buf[j];
				if (buf[j+1]==0 && buf[j+2]==0 && buf[j+3]<2)
				//if (buf[j+1]==0 && buf[j+2]==0 && buf[j+3]==0 && buf[j+4]<2)
					break;//break while，NAL_unit结束
				++j;
			}
			nalsize[tnal] = k - i - 4;
			++tnal;
			i = j;
		}
	}
	
	for (int i = 0; i < tnal; ++i)
	{
		printf("NAL%6d : %08lx %08lx(%5d) REF=%d Type=%2d %s\n", i, nalunits[i]-buf, nalsize[i], nalsize[i], getNalRefIdc(i), getNalType(i), NalTypeNames[getNalType(i)]);
		switch (getNalType(i))//nal_unit_type,u(5),h264标准P73，哪些类型需要switch？
		{
		case 7://序列参数集
			readSequenceParameterSet(i);
			break;
		case 8://图像参数集
			readPictureParameterSet(i);
			break;
		case 1:
		case 5:
			readSliceHeader(i);
			psp[sp.timeId][sp.viewId] = sp;
			break;
		default:
			cout << "not found " << getNalType(i) << endl;
			break;
		}
	}
}

int main()
{
	//cout << "main" <<endl;
	memset(picts, 0, sizeof(picts));
	freopen("3d-432x240-reorder-nocabac-gop64.txt", "wt", stdout);
	init();
	getnalunits();
	printf("Finished!\n");
	return 0;
}
