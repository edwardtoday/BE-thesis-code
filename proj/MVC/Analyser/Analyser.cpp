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

const char *inputFileName = "../golf25.mvc";

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
	FILE *f = fopen(inputFileName, "rb");
	size = fread(buf, 1, sizeof(buf), f);
	fclose(f);
}


void Comment(char *str)
{
	//	printf("%20s  ", str);
}

int ReadCode(uint32_t len)
{
	int code = eg_read_direct(&bs, len);
	//	printf("Code %08lX, %d\n", code, len);
	return code;
}

int ReadFlag()
{
	int code = eg_read_direct1(&bs);
	//	printf("Flag %d\n", code);
	return code;
}

uint32_t ReadUvlc()
{
	uint32_t uvlc = eg_read_ue(&bs);
	//	printf("Uvlc %d\n", uvlc);
	return uvlc;
}

int32_t ReadSvlc()
{
	int32_t svlc = eg_read_se(&bs);
	//	printf("Svlc %d\n", svlc);
	return svlc;
}

void ByteAlign()
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
	eg_read_direct(&bs, 1);
	eg_read_direct(&bs, 2);
	return eg_read_direct(&bs, 5);
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

	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_SPS);

	Comment("SPS: profile_idc");
	ASSERT(ReadCode(8) == HIGH_PROFILE);
	Comment("SPS: constrained_set0_flag");
	ReadFlag();
	Comment("SPS: constrained_set1_flag");
	ReadFlag();
	Comment("SPS: constrained_set2_flag");
	ReadFlag();
	Comment("SPS: constrained_set3_flag");
	ReadFlag();
	Comment("SPS: constrained_set4_flag");
	ReadFlag();
	Comment("SPS: constrained_set5_flag");
	ReadFlag();
	Comment("SPS: reserved_zero_2bits");
	ASSERT(ReadCode(2) == 0);
	Comment("SPS: level_idc");
	int level_idc = ReadCode(8);
	ASSERT(level_idc==13 || level_idc==21);
	Comment("SPS: seq_parameter_set_id");
	ReadUvlc();

	Comment("--- fidelity range extension syntax ---");
	Comment("ReadFrext");
	Comment("SPS: chroma_format_idc");
	ASSERT(ReadUvlc() == 1); // 4:2:0
	Comment("SPS: bit_depth_luma_minus8");
	ASSERT(ReadUvlc() == 0);
	Comment("SPS: bit_depth_chroma_minus8");
	ASSERT(ReadUvlc() == 0);
	Comment("SPS: qpprime_y_zero_transform_bypass_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: seq_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);

	Comment("SPS: log2_max_frame_num_minus4");
	int log2nFramesM4 = ReadUvlc();
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0;
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	int log2nPocM4 = ReadUvlc();

	Comment("SPS: num_ref_frames");
	sps.numRefFrames = ReadUvlc();
	Comment("SPS: required_frame_num_update_behaviour_flag");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: pic_width_in_mbs_minus_1");
	sps.width  = (ReadUvlc() + 1) << 4;
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	sps.height = (ReadUvlc() + 1) << 4;
	Comment("SPS: frame_mbs_only_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: direct_8x8_inference_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);

	/*
	ASSERT(eg_read_direct(&bs, 1)==0);
	eg_read_direct(&bs, 2);
	eg_read_direct(&bs, 5);
	Comment("SPS: profile_idc");
	ASSERT(ReadCode(8) == HIGH_PROFILE);
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
	printf("\tParameter Set Id = %d\n", ReadUvlc());

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

	Comment("SPS: log2_max_frame_num_minus4");
	int log2nFramesM4 = ReadUvlc();
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0; // getPicOrderCntType()
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	ASSERT(ReadUvlc() == 5); // ?

	Comment("SPS: num_ref_frames");
	sps.numRefFrames = ReadUvlc();
	printf("\tNum Ref Frames = %d\n", sps.numRefFrames);
	Comment("SPS: required_frame_num_update_behaviour_flag");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: pic_width_in_mbs_minus_1");
	sps.width  = (ReadUvlc() + 1) << 4;
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	sps.height = (ReadUvlc() + 1) << 4;
	printf("\tSize = (%d,%d)\n", sps.width, sps.height);
	Comment("SPS: frame_mbs_only_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: direct_8x8_inference_flag");
	ASSERT(ReadFlag() == 1);
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);
	*/
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
	ASSERT(ReadCode(8) == MULTI_VIEW_PROFILE);
	Comment("SPS: constrained_set0_flag");
	ReadFlag();
	Comment("SPS: constrained_set1_flag");
	ReadFlag();
	Comment("SPS: constrained_set2_flag");
	ReadFlag();
	Comment("SPS: constrained_set3_flag");
	ReadFlag();
	Comment("SPS: constrained_set4_flag");
	ReadFlag();
	Comment("SPS: constrained_set5_flag");
	ReadFlag();
	Comment("SPS: reserved_zero_2bits");
	ASSERT(ReadCode(2) == 0);
	Comment("SPS: level_idc");
	int level_idc = ReadCode(8);
	ASSERT(level_idc==31 || level_idc==30 || level_idc==21);
	Comment("SPS: seq_parameter_set_id");
	ReadUvlc();

	Comment("--- fidelity range extension syntax ---");
	Comment("ReadFrext");
	Comment("SPS: chroma_format_idc");
	ASSERT(ReadUvlc() == 1);
	Comment("SPS: bit_depth_luma_minus8");
	ASSERT(ReadUvlc() == 0);
	Comment("SPS: bit_depth_chroma_minus8");
	ASSERT(ReadUvlc() == 0);
	Comment("SPS: qpprime_y_zero_transform_bypass_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: seq_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);

	int log2nFramesM4 = ReadUvlc();
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	int picOrderCntType = 0;
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	int log2nPocM4 = ReadUvlc();
	sps.log2nPoc = log2nPocM4 + 4;
	sps.nPocM1 = (1<<sps.log2nPoc) - 1;

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

	sps.viewCodingOrder.clear();
	for (int i = 0; i < sps.viewCount; ++i)
	{
		Comment("SPS: view_id[i]");
		sps.viewCodingOrder.push_back(ReadUvlc());
	}

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

	Comment("SPS: num_level_values_signalled");
	ASSERT(ReadUvlc() == 0);

	/*
	ASSERT(ReadCode(8) == 0);
	ASSERT(ReadUvlc() == 0);
	ASSERT(ReadCode(3) == 0);
	ASSERT(ReadUvlc() == 0);
	ASSERT(ReadUvlc() == 0);
	ASSERT(ReadUvlc() == 0);
	*/

	Comment("SUBSET SPS: mvc_vui_parameters_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SUBSET SPS: Additional_extension2_flag");
	ASSERT(ReadFlag() == 0);


	/*
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
	*/
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
	ReadUvlc();
	Comment("PPS: seq_parameter_set_id");
	ReadUvlc();
	Comment("PPS: entropy_coding_mode_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: pic_order_present_flag");
	ASSERT(ReadFlag() == 1);
	Comment("PPS: num_slice_groups_minus1");
	ASSERT(ReadUvlc() == 0);

	Comment("PPS: num_ref_idx_l0_active_minus1");
	pps.num_ref_idx_l0_active = ReadUvlc() + 1;
	Comment("PPS: num_ref_idx_l1_active_minus1");
	pps.num_ref_idx_l1_active = ReadUvlc() + 1;
	Comment("PPS: weighted_pred_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: weighted_bipred_idc");
	ASSERT(ReadCode(2) == 0);
	Comment("PPS: pic_init_qp_minus26");
	pps.baseQP = ReadSvlc() + 26;
	Comment("PPS: pic_init_qs_minus26");
	ASSERT(ReadSvlc() == 0);
	Comment("PPS: chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);
	Comment("PPS: deblocking_filter_parameters_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: constrained_intra_pred_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: redundant_pic_cnt_present_flag");
	ASSERT(ReadFlag() == 0);

	Comment("PPS: transform_8x8_mode_flag");
	ASSERT(ReadFlag() == 1);
	Comment("PPS: pic_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: second_chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);

	/*
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_PPS);

	Comment("PPS: pic_parameter_set_id");
	ReadUvlc();
	Comment("PPS: seq_parameter_set_id");
	ReadUvlc();
	Comment("PPS: entropy_coding_mode_flag");
	ASSERT(ReadFlag() == 0); // CAVLC, 1 CABAC
	Comment("PPS: pic_order_present_flag");
	ASSERT(ReadFlag() == 1);
	Comment("PPS: num_slice_groups_minus1");
	ASSERT(ReadUvlc() == 0);

	Comment("PPS: num_ref_idx_l0_active_minus1");
	pps.num_ref_idx_l0_active = ReadUvlc() + 1;
	Comment("PPS: num_ref_idx_l1_active_minus1");
	pps.num_ref_idx_l1_active = ReadUvlc() + 1;
	Comment("PPS: weighted_pred_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: weighted_bipred_idc");
	ASSERT(ReadCode(2) == 0);
	Comment("PPS: pic_init_qp_minus26");
	pps.baseQP = ReadSvlc() + 26;
	printf("\tBase QP = %d\n", pps.baseQP);
	Comment("PPS: pic_init_qs_minus26");
	ASSERT(ReadSvlc() == 0);
	Comment("PPS: chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);
	Comment("PPS: deblocking_filter_parameters_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: constrained_intra_pred_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: redundant_pic_cnt_present_flag");
	ASSERT(ReadFlag() == 0);

	Comment("PPS: transform_8x8_mode_flag");
	ASSERT(ReadFlag() == 1);
	Comment("PPS: pic_scaling_matrix_present_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: second_chroma_qp_index_offset");
	ASSERT(ReadSvlc() == 0);
	*/
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
	Comment("NALU HEADER: priority_id");
	sp.priority_id = ReadCode(6);
	Comment("NALU HEADER: view_id");
	sp.viewId = ReadCode(10);
	Comment("NALU HEADER: temporal_id");
	int temporal_id = ReadCode(3);
	Comment("NALU HEADER: anchor_pic_flag");
	sp.isAnchor = (ReadCode(1) == 1);
	Comment("NALU HEADER: inter_view_flag");
	int inter_view = ReadCode(1);
	Comment("NALU HEADER: reserved_zero_one_bit");
	ReadCode(1);

	/*
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
	*/
}

void readSliceHeader(int id)
{
	BitstreamInit(&bs, nalunits[id], nalsize[id]);


	int long_term_reference_flag = 0;
	int adaptive_ref_pic_marking_mode_flag = 0;
	int ref_pic_list_reordering_flag0 = 0;
	int ref_pic_list_reordering_flag1 = 0;
	//SequenceParametersSet &sps = CodecInfo::GetInstance().sps;
	//PictureParametersSet &pps = CodecInfo::GetInstance().pps;
	int num_ref_idx_l0_active = pps.num_ref_idx_l0_active;
	int num_ref_idx_l1_active = pps.num_ref_idx_l1_active;
	vector<int> mc;
	vector<int> mc1;
	vector<int> mc2;
	vector<pair<int, int> > reo0, reo1;

	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	int nal_ref_idc = ReadCode(2);
	sp.isRef = (nal_ref_idc != 0);
	Comment("NALU HEADER: nal_unit_type");
	int nal_unit_type = ReadCode(5);
	if (nal_unit_type == 20)
	{
		Comment("NALU HEADER: svc_mvc_flag");
		ASSERT(ReadFlag() == 0);
		Comment("NALU HEADER: non_idr_flag");
		sp.IDR = (ReadFlag()==0);
		Comment("NALU HEADER: priority_id");
		sp.priority_id = ReadCode(6);
		Comment("NALU HEADER: view_id");
		sp.viewId = ReadCode(10);
		Comment("NALU HEADER: temporal_id");
		int temporal_id = ReadCode(3);
		//		printf("\t\tTemporal Level = %d\n", temporal_id);
		Comment("NALU HEADER: anchor_pic_flag");
		sp.isAnchor = (ReadCode(1) == 1);
		Comment("NALU HEADER: inter_view_flag");
		int inter_view_flag = ReadCode(1);
		//		printf("\t\tInter view flag = %d\n", inter_view_flag);
		Comment("NALU HEADER: reserved_zero_one_bit");
		ReadCode(1);
	}
	Comment("SH: first_mb_in_slice");
	ASSERT(ReadUvlc() == 0);
	Comment("SH: slice_type");
	sp.SliceType = ReadUvlc();
	Comment("SH: pic_parameter_set_id");
	ReadUvlc();
	Comment("SH: frame_num");
	sp.frameNum = ReadCode(sps.log2nFrames);
	if( sp.IDR/* && nal_unit_type==NAL_UNIT_CODED_SLICE_IDR*/ )
	{
		Comment("SH: idr_pic_id");
		int idrPicId = ReadUvlc();
		printf("\tIDR Picture ID = %d\n", idrPicId);
	}
	Comment("SH: pic_order_cnt_lsb");
	sp.timeId = ReadCode(sps.log2nPoc);
	printf(" TIME,VIEW = %d, %d\n", sp.timeId, sp.viewId);
	if (sp.timeId==1 && sp.viewId==1)
	{
		printf("");
	}

	Comment("SH: delta_pic_order_cnt_bottom");
	int delta_pic_order_cnt_bottom = ReadSvlc();
	printf("\tdelta_pic_order_cnt_bottom = %d\n", delta_pic_order_cnt_bottom);

	if (sp.SliceType==1 || sp.SliceType==6) // B, EB
	{
		Comment("SH: direct_spatial_mv_pred_flag");
		int direct_spatial_mv_pred_flag = ReadFlag();
		ASSERT(direct_spatial_mv_pred_flag == 1)
	}

	if (sp.SliceType==0 || sp.SliceType==1 || sp.SliceType==5 || sp.SliceType==6)
	{
		Comment("SH: num_ref_idx_active_override_flag");
		int num_ref_idx_active_override_flag = ReadFlag();
		if (num_ref_idx_active_override_flag)
		{
			Comment("SH: num_ref_idx_l0_active_minus1");
			num_ref_idx_l0_active = ReadUvlc() + 1;
			//			printf("\tNumber REF Index L0 = %d\n", num_ref_idx_l0_active);
			if (sp.SliceType==1 || sp.SliceType==6)
			{
				Comment("SH: num_ref_idx_l1_active_minus1");
				num_ref_idx_l1_active = ReadUvlc() + 1;
				//				printf("\tNumber REF Index L1 = %d\n", num_ref_idx_l1_active);
			}
		}
		Comment("RIR: ref_pic_list_reordering_flag");
		ref_pic_list_reordering_flag0 = ReadFlag();
		//		if (ref_pic_list_reordering_flag0)
		//			printf("\tREF Picture List 0 Reorder\n");
		if (ref_pic_list_reordering_flag0)
		{
			int reordering_of_pic_nums_idc;
			int tt = 10;
			do {
				reordering_of_pic_nums_idc = ReadUvlc();
				int va = 0;
				if (reordering_of_pic_nums_idc != 3)
					va = ReadUvlc();
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
				ASSERT(tt--);
			} while (reordering_of_pic_nums_idc != 3);
		}
		if (sp.SliceType==1 || sp.SliceType==6) // B, EB
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			ref_pic_list_reordering_flag1 = ReadFlag();
			// 			if (ref_pic_list_reordering_flag1)
			// 				printf("\tREF Picture List 1 Reorder\n");
			int tt = 6;
			if (ref_pic_list_reordering_flag1)
			{
				int reordering_of_pic_nums_idc;
				do {
					reordering_of_pic_nums_idc = ReadUvlc();
					int va = 0;
					if (reordering_of_pic_nums_idc != 3)
						va = ReadUvlc();
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
					ASSERT(tt--);
				} while (reordering_of_pic_nums_idc != 3);
			}
		}
	}

	if (nal_ref_idc)
	{
		if (nal_unit_type==5 || sp.IDR)
		{
			Comment("DRPM: no_output_of_prior_pics_flag");
			int no_output_of_prior_pics_flag = ReadFlag();
			printf("\t\tno_output_of_prior_pics_flag = %d\n", no_output_of_prior_pics_flag);
			Comment("DRPM: long_term_reference_flag");
			long_term_reference_flag = ReadFlag();
			printf("\t\tlong_term_reference_flag = %d\n", long_term_reference_flag);
		}
		else
		{
			Comment("adaptive_ref_pic_marking_mode_flag");
			adaptive_ref_pic_marking_mode_flag = ReadFlag();
			if (adaptive_ref_pic_marking_mode_flag)
			{
				int memory_management_control_operation;
				int pb, pa;
				do {
					memory_management_control_operation = ReadUvlc();
					pa = pb = 0;
					switch (memory_management_control_operation)
					{
					case 3:
						pb = ReadUvlc();

					case 1:
					case 2:
					case 4:
					case 6:
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
							ASSERT(0);
						}
					}
					else
					{
						ASSERT(0);
					}

				} while (memory_management_control_operation != 0);
			}
		}
	}

	Comment("SH: slice_qp_delta");
	sp.QP_Delta = ReadSvlc();
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
		; // Nothing
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
		ASSERT(0);
	}



	if (sp.isAnchor)
	{
		for (int i = 0; i < sps.anchorRefsList0[sp.viewId].size(); ++i)
			list0.push_back(make_pair(sp.timeId, sps.anchorRefsList0[sp.viewId][i]));
		for (int i = 0; i < sps.anchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(make_pair(sp.timeId, sps.anchorRefsList1[sp.viewId][i]));
	}
	else
	{
		for (int i = 0; i < sps.nonAnchorRefsList0[sp.viewId].size(); ++i)
			list0.push_back(make_pair(sp.timeId, sps.nonAnchorRefsList0[sp.viewId][i]));
		for (int i = 0; i < sps.nonAnchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(make_pair(sp.timeId, sps.nonAnchorRefsList1[sp.viewId][i]));
	}

#define RPLR_NEG 0
#define RPLR_POS 1
#define RPLR_LONG 2
#define RPLR_END 3
#define RPLR_VIEW_NEG 4
#define RPLR_VIEW_POS 5

	if (ref_pic_list_reordering_flag0)
	{
		UInt  uiPicNumPred    = sp.frameNum;
		UInt  uiIndex         = 0;   
		UInt  uiCommand       = 0;   
		UInt  uiIdentifier    = 0;   
		// JVT-V043   
		UInt  uiPicViewIdx    = -1;   
		for (int i = 0; i < reo0.size(); ++i)
		{   
			uiCommand = reo0[i].first;
			uiIdentifier = reo0[i].second - 1;
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

				//----- find picture in reference list -----   
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



	// ²Î¿¼Í¼Ïñ
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

#if 0
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
	if (nal_unit_type == 20)
	{
		Comment("NALU HEADER: svc_mvc_flag");
		ASSERT(ReadFlag() == 0);
		Comment("NALU HEADER: non_idr_flag");
		sp.IDR = (ReadFlag()==0);
		Comment("NALU HEADER: priority_id");
		sp.priority_id = ReadCode(6);
		Comment("NALU HEADER: view_id");
		sp.viewId = ReadCode(10);
		Comment("NALU HEADER: temporal_id");
		int temporal_id = ReadCode(3);
		printf("\t\tTemporal Level = %d\n", temporal_id);
		Comment("NALU HEADER: anchor_pic_flag");
		sp.isAnchor = (ReadCode(1) == 1);
		Comment("NALU HEADER: inter_view_flag");
		int inter_view_flag = ReadCode(1);
		printf("\t\tInter view flag = %d\n", inter_view_flag);
		Comment("NALU HEADER: reserved_zero_one_bit");
		ASSERT(ReadCode(1) == 0);
	}
	Comment("SH: first_mb_in_slice");
	ASSERT(ReadUvlc() == 0);
	Comment("SH: slice_type");
	sp.SliceType = ReadUvlc();
	Comment("SH: pic_parameter_set_id");
	int ppsid = ReadUvlc();
	ASSERT(ppsid == (nal_unit_type==20));
	Comment("SH: frame_num");
	sp.frameNum = ReadCode(sps.log2nFrames);
	if( sp.IDR && nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )
	{
		Comment("SH: idr_pic_id");
		int idrPicId = ReadUvlc();
		printf("\tIDR Picture ID = %d\n", idrPicId);
	}
	Comment("SH: pic_order_cnt_lsb");
	// Time Id
	sp.timeId = ReadCode(sps.log2nFrames);


#endif
}

void getnalunits()
{
	cout << size << endl;
	tnal = 0;
	for (int i = 0; i < size; ++i)
	{
		if (buf[i]==0 && buf[i+1]==0 && buf[i+2]==0 && buf[i+3]==1)
		{
			nalunits[tnal] = buf + i + 4;
			int j = i + 4, k = j;
			while (j<size)
			{
				if (buf[j-1]==0 && buf[j-2]==0 && buf[j-3]==0 && buf[j]==3)
				{
				}
				else
					buf[k++] = buf[j];
				if (buf[j]==0 && buf[j+1]==0 && buf[j+2]==0 && buf[j+3]<3)
					break;
				++j;
			}
			nalsize[tnal] = k - i;
			++tnal;
			i = j - 1;
		}
	}
	
	for (int i = 0; i < tnal; ++i)
	{
		printf("NAL%6d : %08lx %08lx(%5d) REF=%d Type=%2d %s\n", i, nalunits[i]-buf, nalsize[i], nalsize[i], getNalRefIdc(i), getNalType(i), NalTypeNames[getNalType(i)]);
		switch (getNalType(i))
		{
		case 7:
			readSequenceParameterSet(i);
			break;
		case 15:
			readSubsetSequenceParameterSet(i);
			break;
		case 8:
			readPictureParameterSet(i);
			break;
		case 14: // Prefix NAL unit
			readSlicePrefix(i);
			break;
		case 1:
		case 5:
		case 20:
			readSliceHeader(i);
			psp[sp.timeId][sp.viewId] = sp;
			break;
		}
	}
}

int main()
{
	memset(picts, 0, sizeof(picts));
	freopen("test.txt", "wt", stdout);
	init();
	getnalunits();
	printf("Finished!\n");
	return 0;
}
