#include "NALParser.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <vector>

#pragma warning(disable:4018)
using namespace std;

#undef ASSERT
#define ASSERT(b) { if (!(b)) { printf("Assert Error (%s,%d)!!!!\n", __FILE__, __LINE__); exit(0); } }

typedef struct tagPictRefInfo {
	bool ShortRef;
	int FrameNum;
	int FrameNumWrap;
	int PicNum;
	bool LongRef;
	int LongTermPicNum;
	int MaxLongTermFrameIdx;
} PictRefInfo;

int NALParser::getNalType()
{
	eg_read_direct(&bs, 1);//forbidden_zero_bit
	nal_ref_idc = eg_read_direct(&bs, 2);//nal_ref_idc
	nal_unit_type = eg_read_direct(&bs, 5);//nal_unit_type
	return nal_unit_type;
}

void NALParser::Comment(char *str)
{
	printf("%s  ", str);
}

int NALParser::ReadCode(uint32_t len)//n位直接读取
{
	int code = eg_read_direct(&bs, len);
	printf("Code %08lX, %d\n", code, len);
	return code;
}

int NALParser::ReadFlag()//1位直接读取
{
	int code = eg_read_direct1(&bs);
	printf("Flag %d\n", code);
	return code;
}

uint32_t NALParser::ReadUvlc()//ue(v)
{
	uint32_t uvlc = eg_read_ue(&bs);
	printf("Uvlc %d\n", uvlc);
	return uvlc;
}

int32_t NALParser::ReadSvlc()//se(v)
{
	int32_t svlc = eg_read_se(&bs);
	printf("Svlc %d\n", svlc);
	return svlc;
}

bool NALParser::more_rbsp_data()
{
	int32_t pos = BitstreamPos(&bs);
	int32_t len = bs.length * 8;
	uint8_t last = ((uint8*)bs.start)[bs.length-1];
	if (last==0)
		len-=8;
	else
	{
		while (!(last&1))
		{
			last>>=1;
			--len;
		}
	}
	return pos<len-1;
}

void NALParser::readHrdParameters(hrd_parameters &param)
{
	Comment("cpb_cnt_minus1");
	param.cpb_cnt = ReadUvlc() + 1;
	Comment("bit_rate_scale");
	param.bit_rate_scale = ReadCode(4);
	Comment("cpb_size_scale");
	param.cpb_size_scale = ReadCode(4);
	for (int i = 0; i < param.cpb_cnt; ++i)
	{
		Comment("bit_rate_value_minus1[ SchedSelIdx ]");
		param.bit_rate_value[i] = ReadUvlc() + 1;
		Comment("cpb_size_value_minus1[ SchedSelIdx ]");
		param.cpb_size_value[i] = ReadUvlc() + 1;
		Comment("cbr_flag[ SchedSelIdx ]");
		param.cbr_flag[i] = ReadFlag() != 0;
	}
	Comment("initial_cpb_removal_delay_length_minus1");
	param.initial_cpb_removal_delay_length = ReadCode(5) + 1;
	Comment("cpb_removal_delay_length_minus1");
	param.cpb_removal_delay_length = ReadCode(5) + 1;
	Comment("dpb_output_delay_length_minus1");
	param.dpb_output_delay_length = ReadCode(5) + 1;
	Comment("time_offset_length");
	param.time_offset = ReadCode(5);
}

const int EXTENDED_SAR = 255;

void NALParser::readVuiParameters(vui_parameters &param)
{
	memset(&param, 0, sizeof(param));
	Comment("aspect_ratio_info_present_flag");
	param.aspect_ratio_info_present = ReadFlag() != 0;
	if (param.aspect_ratio_info_present)
	{
		Comment("aspect_ratio_idc");
		param.aspect_ratio_idc = ReadCode(8);
		if (param.aspect_ratio_idc == EXTENDED_SAR)
		{
			Comment("sar_width");
			param.sar_width = ReadCode(16);
			Comment("sar_height");
			param.sar_height = ReadCode(16);
		}
	}
	Comment("overscan_info_present_flag");
	param.overscan_info_present = ReadFlag() != 0;
	if (param.overscan_info_present)
	{
		Comment("overscan_appropriate_flag");
		param.overscan_appropriate = ReadFlag() != 0;
	}
	Comment("video_signal_type_present_flag");
	param.video_signal_type_present = ReadFlag() != 0;
	if (param.video_signal_type_present)
	{
		Comment("video_format");
		param.video_format = ReadCode(3);
		Comment("video_full_range_flag");
		param.video_full_range = ReadFlag() != 0;
		Comment("colour_description_present_flag");
		param.color_description_present = ReadFlag() != 0;
		if (param.color_description_present)
		{
			Comment("colour_primaries");
			param.colour_primaries = ReadCode(8);
			Comment("transfer_characteristics");
			param.transfer_characteristics = ReadCode(8);
			Comment("matrix_coefficients");
			param.matrix_coefficients = ReadCode(8);
		}
	}
	else
		param.video_format = 2;
	Comment("chroma_loc_info_present_flag");
	param.chroma_loc_info_present = ReadFlag() != 0;
	if (param.chroma_loc_info_present)
	{
		Comment("chroma_sample_loc_type_top_field");
		param.chroma_sample_loc_type_top_field = ReadUvlc();
		Comment("chroma_sample_loc_type_bottom_field");
		param.chroma_sample_loc_type_bottom_field = ReadUvlc();
	}
	Comment("timing_info_present_flag");
	param.timing_info_present = ReadFlag() != 0;
	if (param.timing_info_present)
	{
		Comment("num_units_in_tick");
		param.num_units_in_tick = ReadCode(32);
		Comment("time_scale");
		param.time_scale = ReadCode(32);
		Comment("fixed_frame_rate_flag");
		param.fixed_frame_rate = ReadFlag() != 0;
	}
	Comment("nal_hrd_parameters_present_flag");
	param.nal_hrd_parameters_present = ReadFlag() != 0;
	if (param.nal_hrd_parameters_present)
		readHrdParameters(param.nal_hrd_parameters);
	Comment("vcl_hrd_parameters_present_flag");
	param.vcl_hrd_parameters_present = ReadFlag() != 0;
	if (param.vcl_hrd_parameters_present)
		readHrdParameters(param.vcl_hrd_parameters);
	if (param.nal_hrd_parameters_present || param.vcl_hrd_parameters_present)
	{
		Comment("low_delay_hrd_flag");
		param.low_delay_hrd = ReadFlag() != 0;
	}
	Comment("pic_struct_present_flag");
	param.pic_struct_present = ReadFlag() != 0;
	Comment("bitstream_restriction_flag");
	param.bitstream_restriction = ReadFlag() != 0;
	if (param.bitstream_restriction)
	{
		Comment("motion_vectors_over_pic_boundaries_flag");
		param.motion_vectors_over_pic_boundaries_flag = ReadFlag() != 0;
		Comment("max_bytes_per_pic_denom");
		param.max_bytes_per_pic_denom = ReadUvlc();
		Comment("max_bits_per_mb_denom");
		param.max_bits_per_mb_denom = ReadUvlc();
		Comment("log2_max_mv_length_horizontal");
		param.log2_max_mv_length_horizontal = ReadUvlc();
		Comment("log2_max_mv_length_vertical");
		param.log2_max_mv_length_vertical = ReadUvlc();
		Comment("num_reorder_frames");
		param.num_reorder_frames = ReadUvlc();
		Comment("max_dec_frame_buffering");
		param.max_dec_frame_buffering = ReadUvlc();
	}
}

void NALParser::readSequenceParameterSet()
{
	//Comment("NALU HEADER: forbidden_zero_bit");
	//ASSERT(ReadFlag() == 0);
	//Comment("NALU HEADER: nal_ref_idc");
	//ASSERT(ReadCode(2) == 3);
	//Comment("NALU HEADER: nal_unit_type");
	//ASSERT(ReadCode(5) == NAL_UNIT_SPS);

	Comment("SPS: profile_idc");
	//ASSERT(ReadCode(8) == HIGH_PROFILE);
	sps.profileIdc = ReadCode(8);
	Comment("SPS: constrained_set0_flag");
	sps.constrained_set0_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set1_flag");
	sps.constrained_set1_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set2_flag");
	sps.constrained_set2_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set3_flag");
	sps.constrained_set3_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set4_flag");
	sps.constrained_set4_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set5_flag");
	sps.constrained_set5_flag = (ReadFlag()==1);
	Comment("SPS: reserved_zero_2bits");
	ASSERT(ReadCode(2) == 0);
	Comment("SPS: level_idc");
	sps.levelIdc = ReadCode(8);//level_idc=21
	Comment("SPS: seq_parameter_set_id");
	sps.seqParamSetId = ReadUvlc();
	//printf("\tParameter Set Id = %d\n", ReadUvlc());//seq_parameter_set_id

	if (sps.profileIdc == 100 || 
		sps.profileIdc == 110 || 
		sps.profileIdc == 122 || 
		sps.profileIdc == 244 || 
		sps.profileIdc == 44 || 
		sps.profileIdc == 83 || 
		sps.profileIdc == 86 || 
		sps.profileIdc == 118 ||
		sps.profileIdc == 128
		) {
			Comment("--- fidelity range extension syntax ---");
			Comment("ReadFrext");
			Comment("SPS: chroma_format_idc");
			//sps.chromaFormatIdc = ReadUvlc();
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
	}

	Comment("SPS: log2_max_frame_num_minus4");
	int log2nFramesM4 = ReadUvlc();
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0;
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	int log2nPocM4 = ReadUvlc();
	sps.log2nPoc = log2nPocM4 + 4;

	Comment("SPS: num_ref_frames");
	sps.numRefFrames = ReadUvlc();
	Comment("SPS: required_frame_num_update_behaviour_flag");
	sps.gaps_in_frame_num_value_allowed_flag = (ReadFlag()==1);

	Comment("SPS: pic_width_in_mbs_minus_1");
	sps.width  = (ReadUvlc() + 1) << 4;//pic_width_in_mbs_minus1
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	int pic_height_in_map_units_minus1 = ReadUvlc();
	printf("\tSize = (%d,%d)\n", sps.width, sps.height);
	Comment("SPS: frame_mbs_only_flag");
	sps.frame_mbs_only_flag = ReadFlag()==1;
	sps.height = ((2 - sps.frame_mbs_only_flag) * (pic_height_in_map_units_minus1 + 1)) << 4;//pic_height_in_map_units_minus1
	if (!sps.frame_mbs_only_flag)
	{
		Comment("SPS: mb_adaptive_frame_field_flag");
		sps.mb_adaptive_frame_field_flag = ReadFlag() == 1;
	}
	else
		sps.mb_adaptive_frame_field_flag = false;
	//ASSERT(sps.frame_mbs_only_flag);//frame_mbs_only_flag=1
	Comment("SPS: direct_8x8_inference_flag");
	sps.direct_8x8_inference = ReadFlag()==1;
	//ASSERT(ReadFlag() == 1);//direct_8x8_inference_flag=1
	Comment("SPS: frame_cropping_flag");
	bool frame_cropping_flag = ReadFlag() != 0;
	if (frame_cropping_flag)
	{
		Comment("SPS: frame_crop_left_offset");
		sps.crop_left = ReadUvlc();
		Comment("SPS: frame_crop_right_offset");
		sps.crop_right = ReadUvlc();
		Comment("SPS: frame_crop_top_offset");
		sps.crop_top = ReadUvlc();
		Comment("SPS: frame_crop_bottom_offset");
		sps.crop_bottom = ReadUvlc();
	}
	else
	{
		sps.crop_left = 0;
		sps.crop_right = 0;
		sps.crop_top = 0;
		sps.crop_bottom = 0;
	}
	Comment("SPS: vui_parameters_present_flag");
	sps.vui_parameters_present = ReadFlag()!=0;

	if (sps.vui_parameters_present)
	{
		readVuiParameters(sps.vuiparams);
	}
}

void NALParser::readSubsetSequenceParameterSet()
{
	//Comment("NALU HEADER: forbidden_zero_bit");
	//ASSERT(ReadFlag() == 0);
	//Comment("NALU HEADER: nal_ref_idc");
	//ASSERT(ReadCode(2) == 3);
	//Comment("NALU HEADER: nal_unit_type");
	//ASSERT(ReadCode(5) == NAL_UNIT_SUBSET_SPS);

	Comment("SPS: profile_idc");
	ASSERT(ReadCode(8) == MULTI_VIEW_PROFILE);
	Comment("SPS: constrained_set0_flag");
	sps.constrained_set0_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set1_flag");
	sps.constrained_set1_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set2_flag");
	sps.constrained_set2_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set3_flag");
	sps.constrained_set3_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set4_flag");
	sps.constrained_set4_flag = (ReadFlag()==1);
	Comment("SPS: constrained_set5_flag");
	sps.constrained_set5_flag = (ReadFlag()==1);
	Comment("SPS: reserved_zero_2bits");
	ASSERT(ReadCode(2) == 0);
	Comment("SPS: level_idc");
	ReadCode(8);
	Comment("SPS: seq_parameter_set_id");
	ReadUvlc();

	if (sps.profileIdc == 100 || 
		sps.profileIdc == 110 || 
		sps.profileIdc == 122 || 
		sps.profileIdc == 244 || 
		sps.profileIdc == 44 || 
		sps.profileIdc == 83 || 
		sps.profileIdc == 86 || 
		sps.profileIdc == 118 ||
		sps.profileIdc == 128
		) {
			Comment("--- fidelity range extension syntax ---");
			Comment("ReadFrext");
			Comment("SPS: chroma_format_idc");
			//sps.chromaFormatIdc = ReadUvlc();
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
	}

	int log2nFramesM4 = ReadUvlc();
	sps.log2nFrames = log2nFramesM4 + 4;
	sps.nFramesM1 = (1<<sps.log2nFrames)-1;

	Comment("SPS: pic_order_cnt_type");
	int picOrderCntType = 0;
	ASSERT(ReadUvlc() == picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	int log2nPocM4 = ReadUvlc();
	sps.log2nPoc = log2nPocM4 + 4;
	//sps.nPocM1 = (1<<sps.log2nPoc) - 1;

	Comment("SPS: num_ref_frames");
	ReadUvlc();
	Comment("SPS: required_frame_num_update_behaviour_flag");
	ReadFlag();

	Comment("SPS: pic_width_in_mbs_minus_1");
	ReadUvlc();
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	ReadUvlc();
	Comment("SPS: frame_mbs_only_flag");
	ReadFlag();
	Comment("SPS: direct_8x8_inference_flag");
	ReadFlag();
	Comment("SPS: frame_cropping_flag");
	bool frame_cropping_flag = ReadFlag() != 0;
	if (frame_cropping_flag)
	{
		Comment("SPS: frame_crop_left_offset");
		sps.crop_left = ReadUvlc();
		Comment("SPS: frame_crop_right_offset");
		sps.crop_right = ReadUvlc();
		Comment("SPS: frame_crop_top_offset");
		sps.crop_top = ReadUvlc();
		Comment("SPS: frame_crop_bottom_offset");
		sps.crop_bottom = ReadUvlc();
	}
	else
	{
		sps.crop_left = 0;
		sps.crop_right = 0;
		sps.crop_top = 0;
		sps.crop_bottom = 0;
	}
	Comment("SPS: vui_parameters_present_flag");
	sps.vui_parameters_present = ReadFlag()==1;
	if (sps.vui_parameters_present)
	{
		readVuiParameters(sps.vuiparams);
	}

	Comment("SUBSET SPS: bit_equal_to_one");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: num_views_minus_1");
	int viewCount = ReadUvlc() + 1;

	sps.viewCodingOrder.clear();
	for (int i = 0; i < viewCount; ++i)
	{
		Comment("SPS: view_id[i]");
		sps.viewCodingOrder.push_back(ReadUvlc());
	}

	/*
	sps.anchorRefsList0.clear();
	ArrayList<int> tmp;
	for (int i = 0; i < sps.viewCodingOrder.size(); ++i)
	{
		sps.anchorRefsList0.push_back(tmp);
	}
	sps.anchorRefsList1 = sps.anchorRefsList0;
	sps.nonAnchorRefsList0 = sps.anchorRefsList0;
	sps.nonAnchorRefsList1 = sps.anchorRefsList0;
	*/
	for (int i = 1; i < sps.viewCodingOrder.size(); ++i)
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

	for (int i = 1; i < sps.viewCodingOrder.size(); ++i)
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

}

/*
void NALParser::readSequenceParameterSet()
{
	Comment("SPS: profile_idc");
	sps.profileIdc = ReadCode(8);
	//ASSERT(ReadCode(8) == HIGH_PROFILE);//profile_idc=100
	Comment("SPS: constrained_set0_flag");
	sps.constrained_set0_flag = ReadFlag()!=0;
	//ASSERT(ReadFlag() == 0);//constraint_set0_flag=0
	Comment("SPS: constrained_set1_flag");
	sps.constrained_set1_flag = ReadFlag()!=0;
	//ASSERT(ReadFlag() == 0);//constraint_set1_flag=0
	Comment("SPS: constrained_set2_flag");
	sps.constrained_set2_flag = ReadFlag()!=0;
	//ASSERT(ReadFlag() == 0);//constraint_set2_flag=0
	Comment("SPS: constrained_set3_flag");
	sps.constrained_set3_flag = ReadFlag()!=0;
	//ASSERT(ReadFlag() == 0);//constraint_set3_flag=0
	Comment("SPS: constrained_set4_flag");
	sps.constrained_set4_flag = ReadFlag()!=0;
	Comment("SPS: reserved_zero_3bits");
	ReadCode(3);
	//ASSERT(ReadCode(4) == 0);//reserved_zero_4bits=0
	Comment("SPS: level_idc");
	sps.levelIdc = ReadCode(8);//level_idc=21
	Comment("SPS: seq_parameter_set_id");
	sps.seqParamSetId = ReadUvlc();
	//printf("\tParameter Set Id = %d\n", ReadUvlc());//seq_parameter_set_id

	if (sps.profileIdc == 100 || 
		sps.profileIdc == 110 || 
		sps.profileIdc == 122 || 
		sps.profileIdc == 244 || 
		sps.profileIdc == 44 || 
		sps.profileIdc == 83 || 
		sps.profileIdc == 86 || 
		sps.profileIdc == 118 ||
		sps.profileIdc == 128
		) {
		Comment("--- fidelity range extension syntax ---");
		Comment("ReadFrext");
		Comment("SPS: chroma_format_idc");
		//sps.chromaFormatIdc = ReadUvlc();
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
	}

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
	int pic_height_in_map_units_minus1 = ReadUvlc();
	printf("\tSize = (%d,%d)\n", sps.width, sps.height);
	Comment("SPS: frame_mbs_only_flag");
	sps.frame_mbs_only_flag = ReadFlag()==1;
	sps.height = ((2 - sps.frame_mbs_only_flag) * (pic_height_in_map_units_minus1 + 1)) << 4;//pic_height_in_map_units_minus1
	if (!sps.frame_mbs_only_flag)
	{
		Comment("SPS: mb_adaptive_frame_field_flag");
		sps.mb_adaptive_frame_field_flag = ReadFlag() == 1;
	}
	//ASSERT(sps.frame_mbs_only_flag);//frame_mbs_only_flag=1
	Comment("SPS: direct_8x8_inference_flag");
	sps.direct_8x8_inference = ReadFlag()==1;
	//ASSERT(ReadFlag() == 1);//direct_8x8_inference_flag=1
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);//frame_cropping_flag=0
	Comment("SPS: vui_parameters_present_flag");
	sps.vui_parameters_present = ReadFlag()!=0;


	if (sps.vui_parameters_present)
	{
		readVuiParameters(sps.vuiparams);
	}
}
*/

void NALParser::readPictureParameterSet()
{
	Comment("PPS: pic_parameter_set_id");
	pps.picParameterId = ReadUvlc();//pic_parameter_set_id
	Comment("PPS: seq_parameter_set_id");
	pps.setParameterId = ReadUvlc();//seq_parameter_set_id
	Comment("PPS: entropy_coding_mode_flag");
	pps.isEntropyCABAC = ReadFlag()==1; // CAVLC, 1 CABAC entropy_coding_mode_flag=0
	Comment("PPS: pic_order_present_flag");
	pps.isPicOrderPresent = ReadFlag()==1;//pic_order_present_flag=1
	Comment("PPS: num_slice_groups_minus1");
	ASSERT(ReadUvlc() == 0);//num_slice_groups_minus1=0

	Comment("PPS: num_ref_idx_l0_active_minus1");
	pps.num_ref_idx_l0_active = ReadUvlc() + 1;//num_ref_idx_l0_active_minus1
	Comment("PPS: num_ref_idx_l1_active_minus1");
	pps.num_ref_idx_l1_active = ReadUvlc() + 1;//num_ref_idx_l1_active_minus1
	Comment("PPS: weighted_pred_flag");
	pps.weighted_pred_flag = ReadFlag()==1;//weighted_pred_flag=0
	Comment("PPS: weighted_bipred_idc");
	pps.weighted_bipred_idc = ReadCode(2);//weighted_bipred_idc=0
	Comment("PPS: pic_init_qp_minus26");
	pps.baseQP = ReadSvlc() + 26;//pic_init_qp_minus26
	printf("\tBase QP = %d\n", pps.baseQP);
	Comment("PPS: pic_init_qs_minus26");
	ASSERT(ReadSvlc() == 0);//pic_init_qs_minus26
	Comment("PPS: chroma_qp_index_offset");
	pps.QPoffset2 = pps.QPoffset1 = ReadSvlc();
	Comment("PPS: deblocking_filter_parameters_present_flag");
	pps.deblockFilterPresent = (ReadFlag() != 0);
	//deblocking_filter_control_present_flag=0
	Comment("PPS: constrained_intra_pred_flag");
	ASSERT(ReadFlag() == 0);//constrained_intra_pred_flag=0
	Comment("PPS: redundant_pic_cnt_present_flag");
	ASSERT(ReadFlag() == 0);//redundant_pic_cnt_present_flag=0

	//more_rbsp_data()
	if (more_rbsp_data())
	{
		Comment("PPS: transform_8x8_mode_flag");
		pps.transform_8x8_mode_flag = (ReadFlag() == 1);//transform_8x8_mode_flag=1
		Comment("PPS: pic_scaling_matrix_present_flag");
		ASSERT(ReadFlag() == 0);//pic_scaling_matrix_present_flag=0
		Comment("PPS: second_chroma_qp_index_offset");
		pps.QPoffset2 = ReadSvlc();
	}
}

void NALParser::readSlicePrefix(SliceHeader &sp)
{
	//Comment("NALU HEADER: forbidden_zero_bit");
	//ASSERT(ReadFlag() == 0);
	//Comment("NALU HEADER: nal_ref_idc");
	//ReadCode(2);
	//Comment("NALU HEADER: nal_unit_type");
	//ASSERT(ReadCode(5) == NAL_UNIT_CODED_SLICE_PREFIX);
	Comment("NALU HEADER: svc_mvc_flag");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: non_idr_flag");
	sp.IDR = (ReadFlag() == 0);
	Comment("NALU HEADER: priority_id");
	ReadCode(6);
	//sp.priority_id = 
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
}

ref_frame_type makeRefFrameType(int frameNum, int newFrameNum, int timeId, int viewId)
{
	ref_frame_type ret;
	ret.frameNum = frameNum;
	ret.newFrameNum = newFrameNum;
	ret.timeId = timeId;
	ret.viewId = viewId;
	return ret;
}

int NALParser::readSliceHeader(SliceHeader &sp)
{
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
	sp.isRef = nal_ref_idc!=0;
	if (nal_unit_type==20)
	{
		Comment("NALU HEADER: svc_mvc_flag");
		ASSERT(ReadFlag() == 0);
		Comment("NALU HEADER: non_idr_flag");
		sp.IDR = (ReadFlag()==0);
		Comment("NALU HEADER: priority_id");
		ReadCode(6);
		//sp.priority_id = 
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
	sp.first_mb_in_slice = ReadUvlc();
	//ASSERT(ReadUvlc() == 0);//first_mb_in_slice=0
	Comment("SH: slice_type");
	sp.sliceType = ReadUvlc();//slice_type
	//if (sp.sliceType >= 5 )
	//	sp.sliceType -= 5;
	if (sp.sliceType%5==0)
		num_ref_idx_l1_active = 0;
	if (sp.sliceType%5==2)
	{
		num_ref_idx_l0_active = num_ref_idx_l1_active = 0;
	}
	Comment("SH: pic_parameter_set_id");
	int ppsid = ReadUvlc();//pic_parameter_set_id
	//ASSERT(ppsid == (nal_unit_type==20));//ppsid == false?
	Comment("SH: frame_num");
	sp.frameNum = ReadCode(sps.log2nFrames);//frame_num

	static int newFrameNum = -1;
	++newFrameNum;

	if( sp.IDR || nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )
	{
		sp.IDR = true;
		Comment("SH: idr_pic_id");
		int idrPicId = ReadUvlc();
		//printf("\tIDR Picture ID = %d\n", idrPicId);
	}
	Comment("SH: pic_order_cnt_lsb");
	// Time Id
	sp.timeId = ReadCode(sps.log2nPoc);//pic_order_cnt_lsb

	// MVC only
	if (pps.isPicOrderPresent)
	{
		Comment("SH: delta_pic_order_cnt_bottom");
		ReadSvlc();
	}

	if (sp.sliceType==1 || sp.sliceType==6) // B:1,6
	{
		Comment("SH: direct_spatial_mv_pred_flag");
		int direct_spatial_mv_pred_flag = ReadFlag();//direct_spatial_mv_pred_flag
		//ASSERT(direct_spatial_mv_pred_flag == 1)
	}

	if (sp.sliceType==0 || sp.sliceType==1 || sp.sliceType==5 || sp.sliceType==6 || sp.sliceType==3 || sp.sliceType==8 )//P:0,5  SP:3,8  B:1,6
	{
		Comment("SH: num_ref_idx_active_override_flag");
		int num_ref_idx_active_override_flag = ReadFlag();//num_ref_idx_active_override_flag
		if (num_ref_idx_active_override_flag)
		{
			Comment("SH: num_ref_idx_l0_active_minus1");
			num_ref_idx_l0_active = ReadUvlc() + 1;//num_ref_idx_l0_active_minus1
			printf("\tNumber REF Index L0 = %d\n", num_ref_idx_l0_active);
			if (sp.sliceType==1 || sp.sliceType==6)//B
			{
				Comment("SH: num_ref_idx_l1_active_minus1");
				num_ref_idx_l1_active = ReadUvlc() + 1;//num_ref_idx_l1_active_minus1
				printf("\tNumber REF Index L1 = %d\n", num_ref_idx_l1_active);
			}
		}

		//ref_pic_list_reordering()
		Comment("RIR: ref_pic_list_reordering_flag");
		//if( slice_type != I && slice_type != SI )
		ref_pic_list_reordering_flag0 = ReadFlag();//ref_pic_list_reordering_flag_l0
		if (ref_pic_list_reordering_flag0)
			printf("\tREF Picture List 0 Reorder\n");
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
				if (reordering_of_pic_nums_idc==0 || reordering_of_pic_nums_idc==1)
				{
					printf("\t\tabs_diff_pic_num = %d\n", va + 1);
				}
				else if (reordering_of_pic_nums_idc == 2)
				{
					ASSERT(false);
				}
				else if (reordering_of_pic_nums_idc == 4)
				{
					printf("\t\tabs_diff_view_idx = %d\n", va + 1);
				}
				else if (reordering_of_pic_nums_idc == 5)
				{
					printf("\t\tabs_diff_view_idx = %d\n", va + 1);
				}
				tt--;//ASSERT(tt--);
			} while (reordering_of_pic_nums_idc != 3);
		}
		if (sp.sliceType==1 || sp.sliceType==6) // B
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			ref_pic_list_reordering_flag1 = ReadFlag();//ref_pic_list_reordering_flag_l1
			if (ref_pic_list_reordering_flag1)
				printf("\tREF Picture List 1 Reorder\n");
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
					if (reordering_of_pic_nums_idc==0 || reordering_of_pic_nums_idc==1)
					{
						printf("\t\tabs_diff_pic_num = %d\n", va + 1);
					}
					else if (reordering_of_pic_nums_idc == 2)
					{
						ASSERT(false);
					}
					else if (reordering_of_pic_nums_idc == 4)
					{
						printf("\t\tabs_diff_view_idx = %d\n", va + 1);
					}
					else if (reordering_of_pic_nums_idc == 5)
					{
						printf("\t\tabs_diff_view_idx = %d\n", va + 1);
					}
					tt--;//ASSERT(tt--);
				} while (reordering_of_pic_nums_idc != 3);
			}
		}
	}

	if(pps.weighted_pred_flag && sp.sliceType%5==0 || pps.weighted_bipred_idc==1 && sp.sliceType%5==1)
	{
		Comment("luma_log2_weight_denom");
		sp.weightP.luma_log2_weight_denom = ReadUvlc();
		Comment("chroma_log2_weight_denom");
		sp.weightP.chroma_log2_weight_denom = ReadUvlc();
		for (int i = 0; i < num_ref_idx_l0_active; ++i)
		{
			WeightPredItem wpi;
			Comment("luma_weight_l0_flag");
			bool luma_weight_l0_flag = ReadFlag() != 0;
			wpi.flag = luma_weight_l0_flag;
			if (luma_weight_l0_flag)
			{
				Comment("luma_weight_l0[ i ]");
				wpi.weight = ReadSvlc();
				Comment("luma_offset_l0[ i ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.luma0.push_back(wpi);
			Comment("chroma_weight_l0_flag");
			bool chroma_weight_l0_flag = ReadFlag() != 0;
			wpi.flag = chroma_weight_l0_flag;
			if (chroma_weight_l0_flag)
			{
				Comment("chroma_weight_l0[ i ][ 0 ]");
				wpi.weight = ReadSvlc();
				Comment("chroma_offset_l0[ i ][ 0 ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.chromaU0.push_back(wpi);
			if (chroma_weight_l0_flag)
			{
				Comment("chroma_weight_l0[ i ][ 1 ]");
				wpi.weight = ReadSvlc();
				Comment("chroma_offset_l0[ i ][ 1 ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.chromaV0.push_back(wpi);
		}
		for (int i = 0; i < num_ref_idx_l1_active; ++i)
		{
			WeightPredItem wpi;
			Comment("luma_weight_l1_flag");
			bool luma_weight_l1_flag = ReadFlag() != 0;
			wpi.flag = luma_weight_l1_flag;
			if (luma_weight_l1_flag)
			{
				Comment("luma_weight_l1[ i ]");
				wpi.weight = ReadSvlc();
				Comment("luma_offset_l1[ i ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.luma1.push_back(wpi);
			Comment("chroma_weight_l1_flag");
			bool chroma_weight_l1_flag = ReadFlag() != 0;
			wpi.flag = chroma_weight_l1_flag;
			if (chroma_weight_l1_flag)
			{
				Comment("chroma_weight_l1[ i ][ 0 ]");
				wpi.weight = ReadSvlc();
				Comment("chroma_offset_l1[ i ][ 0 ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.chromaU1.push_back(wpi);
			if (chroma_weight_l1_flag)
			{
				Comment("chroma_weight_l1[ i ][ 1 ]");
				wpi.weight = ReadSvlc();
				Comment("chroma_offset_l1[ i ][ 1 ]");
				wpi.offset = ReadSvlc();
			}
			sp.weightP.chromaV1.push_back(wpi);
		}
	}
	/*
	if(pps.weighted_pred_flag && sp.sliceType%5==0 || pps.weighted_bipred_idc==1 && sp.sliceType%5==1)
	{
		Comment("luma_log2_weight_denom");
		ReadUvlc();
		Comment("chroma_log2_weight_denom");
		ReadUvlc();
		if (num_ref_idx_l0_active==5)
		{
			printf("");
		}
		for (int i = 0; i < num_ref_idx_l0_active; ++i)
		{
			Comment("luma_weight_l0_flag");
			bool luma_weight_l0_flag = ReadFlag() != 0;
			if (luma_weight_l0_flag)
			{
				Comment("luma_weight_l0[ i ]");
				ReadSvlc();
				Comment("luma_offset_l0[ i ]");
				ReadSvlc();
			}
			Comment("chroma_weight_l0_flag");
			bool chroma_weight_l0_flag = ReadFlag() != 0;
			if (chroma_weight_l0_flag)
			{
				Comment("chroma_weight_l0[ i ][ 0 ]");
				ReadSvlc();
				Comment("chroma_weight_l0[ i ][ 0 ]");
				ReadSvlc();
				Comment("chroma_weight_l0[ i ][ 1 ]");
				ReadSvlc();
				Comment("chroma_weight_l0[ i ][ 1 ]");
				ReadSvlc();
			}
		}
		for (int i = 0; i < num_ref_idx_l1_active; ++i)
		{
			Comment("luma_weight_l1_flag");
			bool luma_weight_l1_flag = ReadFlag() != 0;
			if (luma_weight_l1_flag)
			{
				Comment("luma_weight_l1[ i ]");
				ReadSvlc();
				Comment("luma_offset_l1[ i ]");
				ReadSvlc();
			}
			Comment("chroma_weight_l1_flag");
			bool chroma_weight_l1_flag = ReadFlag() != 0;
			if (chroma_weight_l1_flag)
			{
				Comment("chroma_weight_l1[ i ][ 0 ]");
				ReadSvlc();
				Comment("chroma_weight_l1[ i ][ 0 ]");
				ReadSvlc();
				Comment("chroma_weight_l1[ i ][ 1 ]");
				ReadSvlc();
				Comment("chroma_weight_l1[ i ][ 1 ]");
				ReadSvlc();
			}
		}
	}
	*/

	if (sp.isRef)
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
					if (memory_management_control_operation==0)
					{

					}
					else if (memory_management_control_operation==1)
					{
						int fn = sp.frameNum - pa - 1;
						bool find = false;
						for (int x = 0; x < ref_frames[sp.viewId].size(); ++x)
							if (ref_frames[sp.viewId][x].frameNum == fn)
							{
								ref_frames[sp.viewId].erase(x);
								find = true;
								break;
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

	if (pps.isEntropyCABAC && sp.sliceType%5!=2)
	{
		Comment("SH: cabac_init_idc");
		sp.cabac_init_idc = ReadUvlc();
	}

	Comment("SH: slice_qp_delta");
	sp.QP_Delta = ReadSvlc();
	if (pps.deblockFilterPresent)
	{
		Comment("SH: disable_deblocking_filter_idc");
		sp.filter.disable_deblocking_filter_idc = ReadUvlc();
		if (sp.filter.disable_deblocking_filter_idc!=1)
		{
			Comment("SH: slice_alpha_c0_offset_div2");
			sp.filter.slice_alpha_c0_offset_div2 = ReadSvlc();
			Comment("SH: slice_beta_offset_div2");
			sp.filter.slice_beta_offset_div2 = ReadSvlc();
		}
	}

	sp.list0.clear();
	sp.list1.clear();

	if (sp.sliceType%5 == 2)
	{
		; // Nothing
	}
	else if (sp.sliceType%5 == 1)
	{
		for (int i = 0; i < ref_frames[sp.viewId].size(); ++i)
			sp.list0.push_back(ref_frames[sp.viewId][i]);
		sp.list1 = sp.list0;
		for (int i = sp.list0.size() - 1; i >= 0; --i)
		{
			for (int j = 0; j < i; ++j)
				if (sp.list0[j].timeId>=sp.timeId && sp.list0[j+1].timeId<sp.timeId ||
					sp.list0[j+1].timeId<sp.timeId && sp.list0[j+1].timeId>sp.list0[j].timeId ||
					sp.list0[j+1].timeId>=sp.timeId && sp.list0[j+1].timeId<sp.list0[j].timeId)
				{
					ref_frame_type tmp = sp.list0[j]; sp.list0[j] = sp.list0[j+1]; sp.list0[j+1] = tmp;
				}
		}
		for (int i = sp.list1.size() - 1; i >= 0; --i)
		{
			for (int j = 0; j < i; ++j)
				if (sp.list1[j].timeId<=sp.timeId && sp.list1[j+1].timeId>sp.timeId ||
					sp.list1[j+1].timeId<=sp.timeId && sp.list1[j+1].timeId>sp.list1[j].timeId ||
					sp.list1[j+1].timeId>sp.timeId && sp.list1[j+1].timeId<sp.list1[j].timeId)
				{
					ref_frame_type tmp = sp.list1[j]; sp.list1[j] = sp.list1[j+1]; sp.list1[j+1] = tmp;
				}
		}
	}
	else if (sp.sliceType%5 == 0)
	{
		for (int i = 0; i < ref_frames[sp.viewId].size(); ++i)
			sp.list0.push_back(ref_frames[sp.viewId][i]);
		for (int i = sp.list0.size() - 1; i >= 0; --i)
			for (int j = 0; j < i; ++j)
			{
				if (sp.list0[j].timeId <
					sp.list0[j+1].timeId)
				{
					ref_frame_type tmp = sp.list0[j]; sp.list0[j] = sp.list0[j+1]; sp.list0[j+1] = tmp;
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
		{
			int viewId = sps.anchorRefsList0[sp.viewId][i];
			for (int j = 0; j < ref_frames[viewId].size(); ++j)
				if (ref_frames[viewId][j].timeId == sp.timeId)
					sp.list0.push_back(ref_frames[viewId][j]);
		}
		for (int i = 0; i < sps.anchorRefsList1[sp.viewId].size(); ++i)
		{
			int viewId = sps.anchorRefsList1[sp.viewId][i];
			for (int j = 0; j < ref_frames[viewId].size(); ++j)
				if (ref_frames[viewId][j].timeId == sp.timeId)
					sp.list1.push_back(ref_frames[viewId][j]);
		}
	}
	else
	{
		for (int i = 0; i < sps.nonAnchorRefsList0[sp.viewId].size(); ++i)
		{
			int viewId = sps.nonAnchorRefsList0[sp.viewId][i];
			for (int j = 0; j < ref_frames[viewId].size(); ++j)
				if (ref_frames[viewId][j].timeId == sp.timeId)
					sp.list0.push_back(ref_frames[viewId][j]);
		}
		for (int i = 0; i < sps.nonAnchorRefsList1[sp.viewId].size(); ++i)
		{
			int viewId = sps.nonAnchorRefsList1[sp.viewId][i];
			for (int j = 0; j < ref_frames[viewId].size(); ++j)
				if (ref_frames[viewId][j].timeId == sp.timeId)
					sp.list1.push_back(ref_frames[viewId][j]);
		}
	}

#define RPLR_NEG 0
#define RPLR_POS 1
#define RPLR_LONG 2
#define RPLR_END 3
#define RPLR_VIEW_NEG 4
#define RPLR_VIEW_POS 5

	if (ref_pic_list_reordering_flag0)
	{
		int  uiPicNumPred    = sp.frameNum;
		int  uiIndex         = 0;   
		int  uiCommand       = 0;   
		int  uiIdentifier    = 0;   
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
				ref_frame_type ft;
				ft.timeId = -1;
				for (int x = uiIndex; x < sp.list0.size(); ++x)
					if (sp.list0[x].frameNum==uiIdentifier && sp.list0[x].viewId==sp.viewId)
					{
						ft = sp.list0[x];
						sp.list0.erase(x);
						break;
					}
				if (ft.timeId>=0)
					sp.list0.insert(uiIndex, ft);
				else
				{
					ASSERT(0);
				}
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


				int targetViewId;
				if (sp.isAnchor)
					targetViewId = sps.anchorRefsList0[sp.viewId][uiIdentifier];
				else
					targetViewId = sps.nonAnchorRefsList0[sp.viewId][uiIdentifier];

				//----- find picture in reference list -----   
				ref_frame_type ft;
				ft.timeId = -1;
				for (int x = 0; x < sp.list0.size(); ++x)
					if (sp.list0[x].timeId==sp.timeId && sp.list0[x].viewId == targetViewId)
					{   
						ft = sp.list0[x];
						sp.list0.erase(x);
						break;
					}  
				if (ft.timeId>=0)
					sp.list0.insert(uiIndex, ft);
				else
				{
					ASSERT(0);
				}
				uiIndex++;   
			} // inter-view   
		} // while   
	}
	if (ref_pic_list_reordering_flag1)
	{
		int  uiPicNumPred    = sp.frameNum;
		int  uiIndex         = 0;   
		int  uiCommand       = 0;   
		int  uiIdentifier    = 0;   
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
				ref_frame_type ft;
				ft.timeId = -1;
				for (int x = uiIndex; x < sp.list0.size(); ++x)
					if (sp.list1[x].frameNum==uiIdentifier && sp.list1[x].viewId==sp.viewId)
					{
						ft = sp.list1[x];
						sp.list1.erase(x);
						break;
					}
				if (ft.timeId>=0)
					sp.list1.insert(uiIndex, ft);
				else
				{
					ASSERT(0);
				}
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


				int targetViewId;
				if (sp.isAnchor)
					targetViewId = sps.anchorRefsList1[sp.viewId][uiIdentifier];
				else
					targetViewId = sps.nonAnchorRefsList1[sp.viewId][uiIdentifier];

				//----- find picture in reference list -----   
				ref_frame_type ft;
				ft.timeId = -1;
				for (int x = 0; x < sp.list1.size(); ++x)
					if (sp.list1[x].timeId == sp.timeId && sp.list1[x].viewId == targetViewId)
					{   
						ft = sp.list1[x];
						sp.list1.erase(x);
						break;
					}   
					sp.list1.insert(uiIndex, ft);

					uiIndex++;   
			} // inter-view    
		} // while   
	}

	while (sp.list0.size() > num_ref_idx_l0_active)
		sp.list0.pop_back();
	while (sp.list1.size() > num_ref_idx_l1_active)
		sp.list1.pop_back();

	// 参考图像
	if (nal_ref_idc)
	{
		if (sp.IDR)
		{
			ref_frames[sp.viewId].clear();
			if (long_term_reference_flag)
			{
				ASSERT(false);
			}
			else
				ref_frames[sp.viewId].push_back(makeRefFrameType(sp.frameNum, newFrameNum, sp.timeId, sp.viewId));
		}
		else
		{
			if (adaptive_ref_pic_marking_mode_flag)//8.2.5.4
			{
				;
			}
			else//8.2.5.3
			{
				int tref = 0;
				int j = sp.viewId;
				if (ref_frames[sp.viewId].size() == sps.numRefFrames)
					ref_frames[sp.viewId].erase(0);
			}
			//if (!picts[sp.timeId][sp.viewId].LongRef)
			{
				ref_frames[sp.viewId].push_back(makeRefFrameType(sp.frameNum, newFrameNum, sp.timeId, sp.viewId));
			}
		}
	}
	else // 8.2.5.2
	{
		// TODO :
	}

	printf("REF 0 Count = %d\n", sp.list0.size());
	for (int i = 0; i < sp.list0.size(); ++i)
	{
		printf(" REF (%d[%d],%d)\n", sp.list0[i].timeId, sp.list0[i].frameNum, sp.list0[i].viewId);
	}
	printf("REF 1 Count = %d\n", sp.list1.size());
	for (int i = 0; i < sp.list1.size(); ++i)
	{
		printf(" REF (%d[%d],%d)\n", sp.list1[i].timeId, sp.list1[i].frameNum, sp.list1[i].viewId);
	}
	/*
	while (!(slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId)))
		Sleep(10);

	CodingPictInfo &info = *slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId);
	info.Ref_Count[0] = sp.list0.size();
	for (int i = 0; i < sp.list0.size(); ++i)
	{
		info.Ref_Id[0][i][0] = sp.list0[i].first;
		info.Ref_Id[0][i][1] = sp.list0[i].second;
		CodingPictInfo *ref = slidingWindow.GetCodingPictInfo(sp.list0[i].first, sp.list0[i].second);
		info.ref_lumaStart[0][i] = ref->lumaStart;
		info.ref_chromaUStart[0][i] = ref->chromaUStart;
		info.ref_chromaVStart[0][i] = ref->chromaVStart;
		info.ref_refVec[0][i] = ref->refVec;
	}
	info.Ref_Count[1] = sp.list1.size();
	for (int i = 0; i < sp.list1.size(); ++i)
	{
		info.Ref_Id[1][i][0] = sp.list1[i].first;
		info.Ref_Id[1][i][1] = sp.list1[i].second;
		CodingPictInfo *ref = slidingWindow.GetCodingPictInfo(sp.list1[i].first, sp.list1[i].second);
		info.ref_lumaStart[1][i] = ref->lumaStart;
		info.ref_chromaUStart[1][i] = ref->chromaUStart;
		info.ref_chromaVStart[1][i] = ref->chromaVStart;
		info.ref_refVec[1][i] = ref->refVec;
	}
	*/
	sp.frameNum = newFrameNum;

	return (bs.tail - bs.start) * 32 + bs.pos;
}





uint8_t* NALParser::Finallize(int &len)
{
	BitstreamFlush(&bs);
	len = (int32_t)(bs.tail-bs.start)*4 + (bs.pos + 7) / 8;
	return (uint8_t*)packerBuf;
}

void NALParser::WriteCode(uint32_t code, uint32_t len)
{
	eg_write_direct(&bs, code, len);
	printf("Code %08lX, %d\n", code, len);
}

void NALParser::WriteFlag(uint32_t code)
{
	eg_write_direct1(&bs, code);
	printf("Flag %d\n", code);
}

void NALParser::WriteUvlc(uint32_t uvlc)
{
	eg_write_ue(&bs, uvlc);
	printf("Uvlc %d\n", uvlc);
}

void NALParser::WriteSvlc(int32_t svlc)
{
	eg_write_se(&bs, svlc);
	printf("Svlc %d\n", svlc);
}

void NALParser::PadByte()
{
	//BitstreamPadAlways(&bs);
	eg_write_direct1(&bs, 1);
	BitstreamPadZero(&bs);
}

int32_t NALParser::writeSequenceParameterSet()
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_SPS, 5);

	Comment("SPS: profile_idc");
	int profileIdc = BASELINE_PROFILE;
	//WriteCode(HIGH_PROFILE, 8);
	WriteCode(profileIdc, 8);
	//	WriteCode(getProfileIdc(), 8);
	Comment("SPS: constrained_set0_flag");
	WriteFlag(sps.constrained_set0_flag);
	Comment("SPS: constrained_set1_flag");
	WriteFlag(sps.constrained_set1_flag);
	Comment("SPS: constrained_set2_flag");
	WriteFlag(sps.constrained_set2_flag);
	Comment("SPS: constrained_set3_flag");
	WriteFlag(sps.constrained_set3_flag);
	Comment("SPS: constrained_set4_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set5_flag");
	WriteFlag(false);
	Comment("SPS: reserved_zero_2bits");
	WriteCode(0, 2);
	Comment("SPS: level_idc");
	WriteCode(sps.levelIdc, 8);
	//	WriteCode(getLevelIdc(), 8);
	Comment("SPS: seq_parameter_set_id");
	static int seq_p_id = 0;
	//WriteUvlc(seq_p_id++);
	WriteUvlc(0);
	// getSeqParameterSetId()

	if (profileIdc == 100 || 
		profileIdc == 110 || 
		profileIdc == 122 || 
		profileIdc == 244 || 
		profileIdc == 44 || 
		profileIdc == 83 || 
		profileIdc == 86 || 
		profileIdc == 118 ||
		profileIdc == 128
		) {
			Comment("--- fidelity range extension syntax ---");
			Comment("WriteFrext");
			Comment("SPS: chroma_format_idc");
			WriteUvlc(1); // 4:2:0
			Comment("SPS: bit_depth_luma_minus8");
			WriteUvlc(0); // BitDepthY = 8 + bit_depth_luma_minus8,  QpBdOffsetY = 6 * bit_depth_luma_minus8
			Comment("SPS: bit_depth_chroma_minus8");
			WriteUvlc(0); // BitDepthC = 8 + bit_depth_chroma_minus8,  QpBdOffsetC = 6 * ( bit_depth_chroma_minus8 + residual_colour_transform_flag)
			Comment("SPS: qpprime_y_zero_transform_bypass_flag");
			WriteFlag(0);
			Comment("SPS: seq_scaling_matrix_present_flag");
			WriteFlag(0);
	}

	Comment("SPS: log2_max_frame_num_minus4");
	WriteUvlc(sps.log2nFrames - 4);

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0; // getPicOrderCntType()
	WriteUvlc(picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	WriteUvlc(sps.log2nPoc - 4); // ?
	// WriteUvlc(getLog2MaxPixOrderCntLsb()-4);

	Comment("SPS: num_ref_frames");
	WriteUvlc(sps.numRefFrames);
	Comment("SPS: gaps_in_frame_num_value_allowed_flag");
	WriteFlag(sps.gaps_in_frame_num_value_allowed_flag); // getRequiredFrameNumupdateBehaviourFlag()

	Comment("SPS: pic_width_in_mbs_minus_1");
	WriteUvlc((sps.width>>4) - 1);
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	WriteUvlc(((sps.height>>4) / (2 - sps.frame_mbs_only_flag)) - 1);
	Comment("SPS: frame_mbs_only_flag");
	WriteFlag(sps.frame_mbs_only_flag);
	if (!sps.frame_mbs_only_flag)
	{
		Comment("SPS: mb_adaptive_frame_field_flag");
		WriteFlag(sps.mb_adaptive_frame_field_flag);
	}
	Comment("SPS: direct_8x8_inference_flag");
	WriteFlag(sps.direct_8x8_inference); // getDirect8x8InferenceFlag()
	Comment("SPS: frame_cropping_flag");
	if (sps.crop_left!=0 || sps.crop_right!=0 || sps.crop_top!=0 || sps.crop_bottom)
	{
		WriteFlag(true);
		Comment("SPS: frame_crop_left_offset");
		WriteUvlc(sps.crop_left);
		Comment("SPS: frame_crop_right_offset");
		WriteUvlc(sps.crop_right);
		Comment("SPS: frame_crop_top_offset");
		WriteUvlc(sps.crop_top);
		Comment("SPS: frame_crop_bottom_offset");
		WriteUvlc(sps.crop_bottom);
	}
	else
	{
		WriteFlag(false);
	}
	Comment("SPS: vui_parameters_present_flag");
	//WriteFlag(0);
	//*
	WriteFlag(sps.vui_parameters_present);
	if (sps.vui_parameters_present)
	{
		writeVuiParameters(sps.vuiparams);
	}
	//*/
	PadByte();
	return seq_p_id - 1;
}













int32_t NALParser::writePictureParameterSet(int seq_parameter_set_id)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_PPS, 5);

	Comment("PPS: pic_parameter_set_id");
	static int pic_p_id = 0;
	//WriteUvlc(pic_p_id++);
	WriteUvlc(0);
	// getPicParameterSetId()
	Comment("PPS: seq_parameter_set_id");
	WriteUvlc(seq_parameter_set_id);
	// getSeqParameterSetId()
	Comment("PPS: entropy_coding_mode_flag");
	WriteFlag(pps.isEntropyCABAC); // CAVLC, 1 CABAC
	Comment("PPS: pic_order_present_flag");
	//pps.isPicOrderPresent = true;
	WriteFlag(pps.isPicOrderPresent);
	// getPicOrderPresentFlag()
	Comment("PPS: num_slice_groups_minus1");
	WriteUvlc(0);

	Comment("PPS: num_ref_idx_l0_active_minus1");
	WriteUvlc(pps.num_ref_idx_l0_active - 1);
	Comment("PPS: num_ref_idx_l1_active_minus1");
	WriteUvlc(pps.num_ref_idx_l1_active - 1);
	Comment("PPS: weighted_pred_flag");
	WriteFlag(pps.weighted_pred_flag);
	Comment("PPS: weighted_bipred_idc");
	WriteCode(pps.weighted_bipred_idc, 2);
	Comment("PPS: pic_init_qp_minus26");
	WriteSvlc(pps.baseQP - 26);
	Comment("PPS: pic_init_qs_minus26");
	WriteSvlc(0);
	Comment("PPS: chroma_qp_index_offset");
	WriteSvlc(pps.QPoffset1);
	// getChomaQpIndexOffset()
	Comment("PPS: deblocking_filter_parameters_present_flag");
	WriteFlag(pps.deblockFilterPresent);
	// *****
	// getDeblockingFilterParametersPresentFlag()
	Comment("PPS: constrained_intra_pred_flag");
	WriteFlag(0);
	// getConstrainedIntraPredFlag()
	Comment("PPS: redundant_pic_cnt_present_flag");
	WriteFlag(0);
	// getRedundantPicCntPresentFlag()

	if (pps.transform_8x8_mode_flag || pps.QPoffset2!=pps.QPoffset1)
	{
		Comment("PPS: transform_8x8_mode_flag");
		WriteFlag(pps.transform_8x8_mode_flag);
		Comment("PPS: pic_scaling_matrix_present_flag");
		WriteFlag(0);
		Comment("PPS: second_chroma_qp_index_offset");
		WriteSvlc(pps.QPoffset2);
	}

	PadByte();
	return pic_p_id - 1;
}

int NALParser::writeSliceHeader(SliceHeader &sp)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	int nal_ref_idc = 3;
	if (!sp.isRef)
		nal_ref_idc = 0;
	WriteCode(nal_ref_idc,   2);
	Comment("NALU HEADER: nal_unit_type");
	int nal_unit_type;
	if (sp.IDR)
		nal_unit_type = NAL_UNIT_CODED_SLICE_IDR;
	else
		nal_unit_type = NAL_UNIT_CODED_SLICE;
	WriteCode(nal_unit_type, 5);
	Comment("SH: first_mb_in_slice");
	//WriteUvlc(0);
	WriteUvlc(sp.first_mb_in_slice);
	Comment("SH: slice_type");
	int slicetype = sp.sliceType;
	//if (slicetype>=2 && slicetype<=4)
	//	slicetype+=5;
	WriteUvlc(slicetype);
	Comment("SH: pic_parameter_set_id");
	WriteUvlc(0);//nal_unit_type==20);
	Comment("SH: frame_num");
	WriteCode(sp.frameNum & sps.nFramesM1, sps.log2nFrames);
	if( nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )
	{
		static int idr_pic_id = 0;
		Comment("SH: idr_pic_id");
		WriteUvlc(idr_pic_id++);
	}
	Comment("SH: pic_order_cnt_lsb");
	WriteCode(sp.timeId & ((1<<sps.log2nPoc)-1), sps.log2nPoc);

	if (pps.isPicOrderPresent)
	{
		Comment("SH: delta_pic_order_cnt_bottom");
		WriteSvlc(0);
	}

	if (sp.sliceType==1 || sp.sliceType==6) // B, EB
	{
		Comment("SH: direct_spatial_mv_pred_flag");
		WriteFlag(1);
	}


	//CodingPictInfo &cpi = *slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId);
	if (sp.sliceType==0 || sp.sliceType==1 || sp.sliceType==5 || sp.sliceType==6)
	{
		int tref0 = sp.list0.size(), tref1 = sp.list1.size();
		Comment("SH: num_ref_idx_active_override_flag");
		if (tref0!=pps.num_ref_idx_l0_active || (sp.sliceType==1 || sp.sliceType==6) && tref1!=pps.num_ref_idx_l1_active)
		{
			WriteFlag(1);
			Comment("SH: num_ref_idx_l0_active_minus1");
			WriteUvlc(tref0 - 1);
			if (sp.sliceType==1 || sp.sliceType==6)
			{
				Comment("SH: num_ref_idx_l1_active_minus1");
				WriteUvlc(tref1 - 1);
			}
		}
		else
		{
			WriteFlag(0);
		}
#define RPLR_NEG 0
#define RPLR_POS 1
#define RPLR_LONG 2
#define RPLR_END 3
#define RPLR_VIEW_NEG 4
#define RPLR_VIEW_POS 5
		Comment("RIR: ref_pic_list_reordering_flag");
		WriteFlag(1);
		int picNum = sp.frameNum;
		int viewIdx = -1;
		for (int i = 0; i < sp.list0.size(); ++i)
		{
			if (sp.list0[i].timeId==sp.timeId)
			{
				int vId = 0;
				if (sp.isAnchor)
				{
					for (int j = 0; j < sps.anchorRefsList0[sp.viewId].size(); ++j)
						if (sps.anchorRefsList0[sp.viewId][j] == sp.list0[i].viewId)
						{
							vId = j;
							break;
						}
				}
				else
				{
					for (int j = 0; j < sps.nonAnchorRefsList0[sp.viewId].size(); ++j)
						if (sps.nonAnchorRefsList0[sp.viewId][j] == sp.list0[i].viewId)
						{
							vId = j;
							break;
						}
				}
				if (vId>viewIdx)
				{
					WriteUvlc(RPLR_VIEW_POS);
					WriteUvlc(vId - viewIdx - 1);
				}
				else if (viewIdx>vId)
				{
					WriteUvlc(RPLR_VIEW_NEG);
					WriteUvlc(viewIdx - vId - 1);
				}
				else
				{
					ASSERT(0);
				}
				viewIdx = vId;
			}
			else
			{
				int frm2write = sp.list0[i].newFrameNum;
				if (frm2write>picNum)
				{
					WriteUvlc(RPLR_POS);
					WriteUvlc(frm2write - picNum - 1);
				}
				else
				{
					WriteUvlc(RPLR_NEG);
					WriteUvlc(picNum - frm2write - 1);
				}
				picNum = frm2write;
			}
		}
		WriteUvlc(RPLR_END);
		if (sp.sliceType==1 || sp.sliceType==6) // B, EB
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			WriteFlag(1);
			int picNum = sp.frameNum;
			int viewIdx = -1;
			for (int i = 0; i < sp.list1.size(); ++i)
			{
				if (sp.list1[i].timeId==sp.timeId)
				{
					int vId = 0;
					if (sp.isAnchor)
					{
						for (int j = 0; j < sps.anchorRefsList1[sp.viewId].size(); ++j)
							if (sps.anchorRefsList1[sp.viewId][j] == sp.list1[i].viewId)
							{
								vId = j;
								break;
							}
					}
					else
					{
						for (int j = 0; j < sps.nonAnchorRefsList1[sp.viewId].size(); ++j)
							if (sps.nonAnchorRefsList1[sp.viewId][j] == sp.list1[i].viewId)
							{
								vId = j;
								break;
							}
					}
					if (vId>viewIdx)
					{
						WriteUvlc(RPLR_VIEW_POS);
						WriteUvlc(vId - viewIdx - 1);
					}
					else if (viewIdx>vId)
					{
						WriteUvlc(RPLR_VIEW_NEG);
						WriteUvlc(viewIdx - vId - 1);
					}
					else
					{
						ASSERT(0);
					}
					viewIdx = vId;
				}
				else
				{
					if (sp.list1[i].newFrameNum>picNum)
					{
						WriteUvlc(RPLR_POS);
						WriteUvlc(sp.list1[i].newFrameNum - picNum - 1);
					}
					else
					{
						WriteUvlc(RPLR_NEG);
						WriteUvlc(picNum - sp.list1[i].newFrameNum - 1);
					}
					picNum = sp.list1[i].newFrameNum;
				}
			}
			WriteUvlc(RPLR_END);
		}
	}


	if(pps.weighted_pred_flag && sp.sliceType%5==0 || pps.weighted_bipred_idc==1 && sp.sliceType%5==1)
	{
		Comment("luma_log2_weight_denom");
		WriteUvlc(sp.weightP.luma_log2_weight_denom);
		Comment("chroma_log2_weight_denom");
		WriteUvlc(sp.weightP.chroma_log2_weight_denom);
		for (int i = 0; i < sp.list0.size(); ++i)
		{
			Comment("luma_weight_l0_flag");
			bool luma_weight_l0_flag = sp.weightP.luma0[i].flag;
			WriteFlag(luma_weight_l0_flag);
			if (luma_weight_l0_flag)
			{
				Comment("luma_weight_l0[ i ]");
				WriteSvlc(sp.weightP.luma0[i].weight);
				Comment("luma_offset_l0[ i ]");
				WriteSvlc(sp.weightP.luma0[i].offset);
			}
			Comment("chroma_weight_l0_flag");
			bool chroma_weight_l0_flag = sp.weightP.chromaU0[i].flag;
			WriteFlag(chroma_weight_l0_flag);
			if (chroma_weight_l0_flag)
			{
				Comment("chroma_weight_l0[ i ][ 0 ]");
				WriteSvlc(sp.weightP.chromaU0[i].weight);
				Comment("chroma_offset_l0[ i ][ 0 ]");
				WriteSvlc(sp.weightP.chromaU0[i].offset);

				Comment("chroma_weight_l0[ i ][ 1 ]");
				WriteSvlc(sp.weightP.chromaV0[i].weight);
				Comment("chroma_offset_l0[ i ][ 1 ]");
				WriteSvlc(sp.weightP.chromaV0[i].offset);
			}
		}
		for (int i = 0; i < sp.list1.size(); ++i)
		{
			Comment("luma_weight_l1_flag");
			bool luma_weight_l1_flag = sp.weightP.luma1[i].flag;
			WriteFlag(luma_weight_l1_flag);
			if (luma_weight_l1_flag)
			{
				Comment("luma_weight_l1[ i ]");
				WriteSvlc(sp.weightP.luma1[i].weight);
				Comment("luma_offset_l1[ i ]");
				WriteSvlc(sp.weightP.luma1[i].offset);
			}
			Comment("chroma_weight_l1_flag");
			bool chroma_weight_l1_flag = sp.weightP.chromaU1[i].flag;
			WriteFlag(chroma_weight_l1_flag);
			if (chroma_weight_l1_flag)
			{
				Comment("chroma_weight_l1[ i ][ 0 ]");
				WriteSvlc(sp.weightP.chromaU1[i].weight);
				Comment("chroma_offset_l1[ i ][ 0 ]");
				WriteSvlc(sp.weightP.chromaU1[i].offset);

				Comment("chroma_weight_l1[ i ][ 1 ]");
				WriteSvlc(sp.weightP.chromaV1[i].weight);
				Comment("chroma_offset_l1[ i ][ 1 ]");
				WriteSvlc(sp.weightP.chromaV1[i].offset);
			}
		}
	}


	if (nal_ref_idc)
	{
		if (nal_unit_type==5 || sp.IDR)
		{
			Comment("DRPM: no_output_of_prior_pics_flag");
			WriteFlag(0);
			Comment("DRPM: long_term_reference_flag");
			WriteFlag(0);
		}
		else
		{
			Comment("adaptive_ref_pic_marking_mode_flag");
			WriteFlag(0);
			// TODO : Memory control operation
		}
	}

	if (pps.isEntropyCABAC && sp.sliceType%5!=2)
	{
		WriteUvlc(sp.cabac_init_idc);
	}

	Comment("SH: slice_qp_delta");
	WriteSvlc(sp.QP_Delta);

	if (pps.deblockFilterPresent)
	{
		Comment("SH: disable_deblocking_filter_idc");
		WriteUvlc(sp.filter.disable_deblocking_filter_idc);
		if (sp.filter.disable_deblocking_filter_idc!=1)
		{
			Comment("SH: slice_alpha_c0_offset_div2");
			WriteSvlc(sp.filter.slice_alpha_c0_offset_div2);
			Comment("SH: slice_beta_offset_div2");
			WriteSvlc(sp.filter.slice_beta_offset_div2);
		}
	}

	return (bs.tail - bs.start) * 32 + bs.pos;
}

void NALParser::writeHrdParameters(hrd_parameters &param)
{
	Comment("cpb_cnt_minus1");
	WriteUvlc(param.cpb_cnt - 1);
	Comment("bit_rate_scale");
	WriteCode(param.bit_rate_scale, 4);
	Comment("cpb_size_scale");
	WriteCode(param.cpb_size_scale, 4);
	for (int i = 0; i < param.cpb_cnt; ++i)
	{
		Comment("bit_rate_value_minus1[ SchedSelIdx ]");
		WriteUvlc(param.bit_rate_value[i] - 1);
		Comment("cpb_size_value_minus1[ SchedSelIdx ]");
		WriteUvlc(param.cpb_size_value[i] - 1);
		Comment("cbr_flag[ SchedSelIdx ]");
		WriteFlag(param.cbr_flag[i]);
	}
	Comment("initial_cpb_removal_delay_length_minus1");
	WriteCode(param.initial_cpb_removal_delay_length - 1, 5);
	Comment("cpb_removal_delay_length_minus1");
	WriteCode(param.cpb_removal_delay_length - 1, 5);
	Comment("dpb_output_delay_length_minus1");
	WriteCode(param.dpb_output_delay_length - 1, 5);
	Comment("time_offset_length");
	WriteCode(param.time_offset, 5);
}

void NALParser::writeVuiParameters(vui_parameters &param)
{
	//memset(&param, 0, sizeof(param));
	Comment("aspect_ratio_info_present_flag");
	WriteFlag(param.aspect_ratio_info_present);
	if (param.aspect_ratio_info_present)
	{
		Comment("aspect_ratio_idc");
		WriteCode(param.aspect_ratio_idc, 8);
		if (param.aspect_ratio_idc == EXTENDED_SAR)
		{
			Comment("sar_width");
			WriteCode(param.sar_width, 16);
			Comment("sar_height");
			WriteCode(param.sar_height, 16);
		}
	}
	Comment("overscan_info_present_flag");
	WriteFlag(param.overscan_info_present);
	if (param.overscan_info_present)
	{
		Comment("overscan_appropriate_flag");
		WriteFlag(param.overscan_appropriate);
	}
	Comment("video_signal_type_present_flag");
	WriteFlag(param.video_signal_type_present);
	if (param.video_signal_type_present)
	{
		Comment("video_format");
		WriteCode(param.video_format, 3);
		Comment("video_full_range_flag");
		WriteFlag(param.video_full_range);
		Comment("colour_description_present_flag");
		WriteFlag(param.color_description_present);
		if (param.color_description_present)
		{
			Comment("colour_primaries");
			WriteCode(param.colour_primaries, 8);
			Comment("transfer_characteristics");
			WriteCode(param.transfer_characteristics, 8);
			Comment("matrix_coefficients");
			WriteCode(param.matrix_coefficients, 8);
		}
	}
	else
	{
		//param.video_format = 2;
	}
	Comment("chroma_loc_info_present_flag");
	WriteFlag(param.chroma_loc_info_present);
	if (param.chroma_loc_info_present)
	{
		Comment("chroma_sample_loc_type_top_field");
		WriteUvlc(param.chroma_sample_loc_type_top_field);
		Comment("chroma_sample_loc_type_bottom_field");
		WriteUvlc(param.chroma_sample_loc_type_bottom_field);
	}
	Comment("timing_info_present_flag");
	WriteFlag(param.timing_info_present);
	if (param.timing_info_present)
	{
		Comment("num_units_in_tick");
		WriteCode(param.num_units_in_tick, 32);
		Comment("time_scale");
		WriteCode(param.time_scale, 32);
		Comment("fixed_frame_rate_flag");
		WriteFlag(param.fixed_frame_rate);
	}
	Comment("nal_hrd_parameters_present_flag");
	WriteFlag(param.nal_hrd_parameters_present);
	if (param.nal_hrd_parameters_present)
		readHrdParameters(param.nal_hrd_parameters);
	Comment("vcl_hrd_parameters_present_flag");
	WriteFlag(param.vcl_hrd_parameters_present);
	if (param.vcl_hrd_parameters_present)
		readHrdParameters(param.vcl_hrd_parameters);
	if (param.nal_hrd_parameters_present || param.vcl_hrd_parameters_present)
	{
		Comment("low_delay_hrd_flag");
		WriteFlag(param.low_delay_hrd);
	}
	Comment("pic_struct_present_flag");
	WriteFlag(param.pic_struct_present);
	Comment("bitstream_restriction_flag");
	WriteFlag(param.bitstream_restriction);
	if (param.bitstream_restriction)
	{
		Comment("motion_vectors_over_pic_boundaries_flag");
		WriteFlag(param.motion_vectors_over_pic_boundaries_flag);
		Comment("max_bytes_per_pic_denom");
		WriteUvlc(param.max_bytes_per_pic_denom);
		Comment("max_bits_per_mb_denom");
		WriteUvlc(param.max_bits_per_mb_denom);
		Comment("log2_max_mv_length_horizontal");
		WriteUvlc(param.log2_max_mv_length_horizontal);
		Comment("log2_max_mv_length_vertical");
		WriteUvlc(param.log2_max_mv_length_vertical);
		Comment("num_reorder_frames");
		WriteUvlc(param.num_reorder_frames);
		Comment("max_dec_frame_buffering");
		WriteUvlc(param.max_dec_frame_buffering);
	}
}

void PadBits(void *buf1, int bl, void *src1, int sl, int size)
{
	uint32_t *buf = (uint32_t*)buf1;
	uint32_t *src = (uint32_t*)src1;
	buf += bl / 32;
	bl &= 31;
	src += sl / 32;
	sl &= 31;
	if (sl != 0)
	{
		uint32_t nd = 32-bl;
		uint32_t old = *buf;
#ifndef ARCH_IS_BIG_ENDIAN
		BSWAP(old);
#endif
		old = old >> nd;
		uint32_t hv = 32-sl;
		uint32_t now = *src;
#ifndef ARCH_IS_BIG_ENDIAN
		BSWAP(now);
#endif
		if (nd >= hv)
		{
			uint32_t and = (1<<hv)-1;
			old = (old<<hv)|(now&and);
			bl += hv;
			if (bl<32)
				old = old << (32-bl);
#ifndef ARCH_IS_BIG_ENDIAN
			BSWAP(old);
#endif
			*buf = old;
			if (bl==32)
			{
				++buf;
				bl = 0;
			}
			++src;
			sl = 0;
		}
		else
		{
			uint32_t and = (1<<nd)-1;
			old = (old<<nd)|((now>>(hv-nd))&and);
#ifndef ARCH_IS_BIG_ENDIAN
			BSWAP(old);
#endif
			*buf = old;
			++buf;
			and = (1<<(hv-nd))-1;
			old = (now & and) << (32 - (hv-nd));
#ifndef ARCH_IS_BIG_ENDIAN
			BSWAP(old);
#endif
			*buf = old;
			bl = hv - nd;
			++src;
			sl = 0;
		}
	}

	if (bl == 0)
	{
		memcpy(buf, src, (size + 7) / 8);
		return ;
	}
	uint32_t nd = 32 - bl;
	uint32_t left = *buf;
#ifndef ARCH_IS_BIG_ENDIAN
	BSWAP(left);
#endif
	left = left >> nd;
	uint32_t and = (1<<bl) - 1;
	size += bl;
	while (size > 0)
	{
		uint32_t v = *(src++);
#ifndef ARCH_IS_BIG_ENDIAN
		BSWAP(v);
#endif
		uint32_t tmp = (left << nd) | (v >> bl);;
#ifndef ARCH_IS_BIG_ENDIAN
		BSWAP(tmp);
#endif
		*(buf++) = tmp;
		left = v & and;
		size -= 32;
	}
}



void NALParser::appendNAL(char *buf, int size)
{
	memset(&sp, 0, sizeof(sp));
	sp.viewId = 0;
	BitstreamInit(&bs, buf, size);
	int type = getNalType();
	switch (type)
	{
	case NAL_UNIT_SPS:
		cerr << "NAL_UNIT_SPS" << endl;
		readSequenceParameterSet();
		state = 1000;
		break;
	case NAL_UNIT_SUBSET_SPS:
		cerr << "NAL_UNIT_SUBSET_SPS" << endl;
		readSubsetSequenceParameterSet();
		break;
	case NAL_UNIT_PPS:
		cerr << "NAL_UNIT_PPS" << endl;
		readPictureParameterSet();
		state = 2000;
		break;
	case NAL_UNIT_CODED_SLICE_PREFIX:
		readSlicePrefix(sp);
		break;
	case NAL_UNIT_CODED_SLICE:
	case NAL_UNIT_CODED_SLICE_IDR:
	case NAL_UNIT_CODED_SLICE_SCALABLE:
	case NAL_UNIT_CODED_SLICE_IDR_SCALABLE:
		cerr << "NAL_UNIT_CODED_SLICE(_IDR)" << endl;
		lastCsize = size;
		memcpy(lastC, buf, size);
		memset(lastC+size, 0, 4);
		lastCstart = readSliceHeader(sp);

		{
			static int curFrame = 0;
			sp.frameNum = curFrame++;
		}

		// TODO: 两路=>多路
		cout << "F(" << sp.timeId << "," << sp.viewId << ")" << ":";
		for (int i = 0; i < sp.list0.size(); ++i)
			cout << "(" << sp.list0[i].timeId << "," << sp.list0[i].viewId << "," << sp.list0[i].frameNum << "," << sp.list0[i].newFrameNum << "),";
		for (int i = 0; i < sp.list1.size(); ++i)
			cout << "(" << sp.list1[i].timeId << "," << sp.list1[i].viewId << "," << sp.list1[i].frameNum << "," << sp.list1[i].newFrameNum << "),";
		cout << endl;
		sp.timeId = sp.timeId + sp.viewId * 2;
		sp.viewId = 0;

		for (int i = 0; i < sp.list0.size(); ++i)
		{
			sp.list0[i] = makeRefFrameType(sp.list0[i].frameNum, sp.list0[i].newFrameNum, sp.list0[i].timeId+sp.list0[i].viewId*2, 0);
		}

		for (int i = 0; i < sp.list1.size(); ++i)
		{
			sp.list1[i] = makeRefFrameType(sp.list1[i].frameNum, sp.list1[i].newFrameNum, sp.list1[i].timeId+sp.list1[i].viewId*2, 0);
		}

		cout << "A(" << sp.timeId << "," << sp.viewId << ")" << ":";
		for (int i = 0; i < sp.list0.size(); ++i)
			cout << "(" << sp.list0[i].timeId << "," << sp.list0[i].viewId << "," << sp.list0[i].frameNum << "," << sp.list0[i].newFrameNum << "),";
		for (int i = 0; i < sp.list1.size(); ++i)
			cout << "(" << sp.list1[i].timeId << "," << sp.list1[i].viewId << "," << sp.list1[i].frameNum << "," << sp.list1[i].newFrameNum << "),";
		cout << endl;
		sp.timeId = sp.timeId + sp.viewId * 2;
		sp.viewId = 0;

		state = 3000;
		break;
	case NAL_UNIT_ACCESS_UNIT_DELIMITER:
		cerr << "NAL_UNIT_ACCESS_UNIT_DELIMITER" << endl;
		printf("TYPE = NAL_UNIT_ACCESS_UNIT_DELIMITER, size=%d\n", size);
		lastCsize = size;
		memcpy(lastC, buf, size);
		memset(lastC+size, 0, 4);
		lastCstart = 0;
		state = 4000;
		break;
	case NAL_UNIT_SEI:
		cerr << "NAL_UNIT_SEI" << endl;
		printf("TYPE = NAL_UNIT_SEI, size=%d\n", size);
		lastCsize = size;
		memcpy(lastC, buf, size);
		memset(lastC+size, 0, 4);
		lastCstart = 0;
		state = 4000;
		break;
	case NAL_UNIT_EXTERNAL:
	case NAL_UNIT_CODED_SLICE_DATAPART_A:
	case NAL_UNIT_CODED_SLICE_DATAPART_B:
	case NAL_UNIT_CODED_SLICE_DATAPART_C:
	case NAL_UNIT_END_OF_SEQUENCE:
	case NAL_UNIT_END_OF_STREAM:
	case NAL_UNIT_FILLER_DATA:
	default:
		cerr << "OTHER" << endl;
		printf("TYPE = %d\n", type);
		ASSERT(0);
	}
}

void NALParser::setMediaInfo(char *buf, int size, MediaInfo &mediaInfo) {
	memset(&sp, 0, sizeof(sp));
	sp.viewId = 0;
	BitstreamInit(&bs, buf, size);
	readSliceHeader(sp);
	mediaInfo.viewId = sp.viewId;
}