#include "Stream/InStream.h"
#include "Codec/Consts4Standard.h"
#include "Configure.h"
#include "ArrayList.h"
#include "Pair.h"
#include "CodecInfo.h"
#include "SlidingWindow.h"
#include "Arch/Arch.h"

InStream::InStream(uint8_t *buf, int buflen) : seq_p_id(0), pic_p_id(0)
{
	BitstreamInit(&bs, buf, buflen);
}

InStream::~InStream()
{
}

void InStream::Init(uint8_t *buf, int buflen)
{
	BitstreamInit(&bs, buf, buflen);
}

void InStream::Comment(char *str)
{
	//printf("%s  ", str);
}

int InStream::ReadCode(uint32_t len)
{
	int code = eg_read_direct(&bs, len);
	//printf("Code %08lX, %d\n", code, len);
	return code;
}

int InStream::ReadFlag()
{
	int code = eg_read_direct1(&bs);
	//printf("Flag %d\n", code);
	return code;
}

uint32_t InStream::ReadUvlc()
{
	uint32_t uvlc = eg_read_ue(&bs);
	//printf("Uvlc %d\n", uvlc);
	return uvlc;
}

int32_t InStream::ReadSvlc()
{
	int32_t svlc = eg_read_se(&bs);
	//printf("Svlc %d\n", svlc);
	return svlc;
}

void InStream::ByteAlign()
{
	BitstreamByteAlign(&bs);
}

void InStream::ResetBitstream()
{
	BitstreamReset(&bs);
}

bool InStream::more_rbsp_data()
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

int InStream::GetNalType()
{
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ReadCode(2);
	Comment("NALU HEADER: nal_unit_type");
	int ret = ReadCode(5);
	ResetBitstream();
	return ret;
}

void InStream::readHrdParameters(hrd_parameters &param)
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

void InStream::readVuiParameters(vui_parameters &param)
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

void InStream::ReadSequenceParameterSet(SequenceParametersSet &sps)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_SPS);

	Comment("SPS: profile_idc");
	//ASSERT(ReadCode(8) == HIGH_PROFILE);
	sps.profileIdc = ReadCode(8);
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
	ReadFlag();

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
	ASSERT(ReadFlag() == 0);//frame_cropping_flag=0
	Comment("SPS: vui_parameters_present_flag");
	sps.vui_parameters_present = ReadFlag()!=0;

	if (sps.vui_parameters_present)
	{
		readVuiParameters(sps.vuiparams);
	}
}

void InStream::ReadSubsetSequenceParameterSet(SequenceParametersSet &sps)
{
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
	sps.nPocM1 = (1<<sps.log2nPoc) - 1;

	Comment("SPS: num_ref_frames");
	ReadUvlc();
	Comment("SPS: required_frame_num_update_behaviour_flag");
	ASSERT(ReadFlag() == 1);

	Comment("SPS: pic_width_in_mbs_minus_1");
	ReadUvlc();
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	ReadUvlc();
	Comment("SPS: frame_mbs_only_flag");
	ReadFlag();
	Comment("SPS: direct_8x8_inference_flag");
	ReadFlag();
	Comment("SPS: frame_cropping_flag");
	ASSERT(ReadFlag() == 0);
	Comment("SPS: vui_parameters_present_flag");
	sps.vui_parameters_present = ReadFlag()==1;
	if (sps.vui_parameters_present)
	{
		readVuiParameters(sps.vuiparams);
	}

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
	sps.readOK = true;
}

void InStream::ReadPictureParameterSet(PictureParametersSet &pps)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	ASSERT(ReadCode(2) == 3);
	Comment("NALU HEADER: nal_unit_type");
	ASSERT(ReadCode(5) == NAL_UNIT_PPS);

	Comment("PPS: pic_parameter_set_id");
	pps.picParameterId = ReadUvlc();//pic_parameter_set_id
	Comment("PPS: seq_parameter_set_id");
	pps.setParameterId = ReadUvlc();//seq_parameter_set_id
	Comment("PPS: entropy_coding_mode_flag");
	pps.cabac = (ReadFlag() != 0);
	//ASSERT(ReadFlag() == 0);
	Comment("PPS: pic_order_present_flag");
	pps.pic_order_present_flag = (ReadFlag() == 1);

	Comment("PPS: num_slice_groups_minus1");
	ASSERT(ReadUvlc() == 0);

	Comment("PPS: num_ref_idx_l0_active_minus1");
	pps.num_ref_idx_l0_active = ReadUvlc() + 1;
	Comment("PPS: num_ref_idx_l1_active_minus1");
	pps.num_ref_idx_l1_active = ReadUvlc() + 1;
	Comment("PPS: weighted_pred_flag");
	pps.weighted_pred_flag = ReadFlag();
	ASSERT(pps.weighted_pred_flag==0);
	Comment("PPS: weighted_bipred_idc");
	pps.weighted_bipred_idc = ReadCode(2);
	ASSERT(pps.weighted_bipred_idc==0);
	Comment("PPS: pic_init_qp_minus26");
	pps.baseQP = ReadSvlc() + 26;
	Comment("PPS: pic_init_qs_minus26");
	ASSERT(ReadSvlc() == 0);
	Comment("PPS: chroma_qp_index_offset");
	pps.QP_offset1 = ReadSvlc();
	Comment("PPS: deblocking_filter_parameters_present_flag");
	pps.deblock_filter_present = (ReadFlag() != 0);
	Comment("PPS: constrained_intra_pred_flag");
	ASSERT(ReadFlag() == 0);
	Comment("PPS: redundant_pic_cnt_present_flag");
	ASSERT(ReadFlag() == 0);

	if (more_rbsp_data())
	{
		Comment("PPS: transform_8x8_mode_flag");
		pps.transform8x8Flag = (ReadFlag() == 1);
		Comment("PPS: pic_scaling_matrix_present_flag");
		ASSERT(ReadFlag() == 0);
		Comment("PPS: second_chroma_qp_index_offset");
		pps.QP_offset2 = ReadSvlc();
	}
}

void InStream::ReadSlicePrefix(SliceParameters &sp, const SequenceParametersSet &sps)
{
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
}

int InStream::ReadSliceHeader(SliceParameters &sp, ArrayList<int> * &ref_frames, SlidingWindow &slidingWindow, int minTime)
{
	int long_term_reference_flag = 0;
	int adaptive_ref_pic_marking_mode_flag = 0;
	int ref_pic_list_reordering_flag0 = 0;
	int ref_pic_list_reordering_flag1 = 0;
	SequenceParametersSet &sps = CodecInfo::GetInstance().sps;
	PictureParametersSet &pps = CodecInfo::GetInstance().pps;
	int num_ref_idx_l0_active = pps.num_ref_idx_l0_active;
	int num_ref_idx_l1_active = pps.num_ref_idx_l1_active;

	Comment("NALU HEADER: forbidden_zero_bit");
	ASSERT(ReadFlag() == 0);
	Comment("NALU HEADER: nal_ref_idc");
	int nal_ref_idc = ReadCode(2);
	sp.isRef = (nal_ref_idc != 0);
	Comment("NALU HEADER: nal_unit_type");
	int nal_unit_type = ReadCode(5);
	if (nal_unit_type==20)
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
	sp.first_mb_in_slice = ReadUvlc();
	Comment("SH: slice_type");
	sp.SliceType = ReadUvlc();
	if (sp.SliceType>=5)
		sp.SliceType -= 5;
	if (sp.SliceType==0)
		num_ref_idx_l1_active = 0;
	if (sp.SliceType==2)
	{
		num_ref_idx_l0_active = num_ref_idx_l1_active = 0;
	}
	Comment("SH: pic_parameter_set_id");
	ReadUvlc();
	Comment("SH: frame_num");
	sp.frameNum = ReadCode(sps.log2nFrames);

	if( sp.IDR || nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )
	{
		Comment("SH: idr_pic_id");
		int idrPicId = ReadUvlc();
		//printf("\tIDR Picture ID = %d\n", idrPicId);
	}
	Comment("SH: pic_order_cnt_lsb");
	sp.timeId = ReadCode(sps.log2nPoc);
	//sp.timeId /= 2;
	if (sp.timeId < minTime)
	{
		sp.timeId |= minTime & ~sps.nPocM1;
		if (sp.timeId < minTime)
			sp.timeId += (1<<sps.log2nPoc);
	}
	//printf(" TIME,VIEW = %d, %d\n", sp.timeId, sp.viewId);
	//if (sp.timeId==1 && sp.viewId==1)
	//{
	//	printf("");
	//}
	if (pps.pic_order_present_flag)
	{
		Comment("SH: delta_pic_order_cnt_bottom");
		ASSERT(ReadSvlc() == 0);
	}
	ArrayList<Pair<int, int> > reo0, reo1;
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
			if (sp.SliceType==1 || sp.SliceType==6)
			{
				Comment("SH: num_ref_idx_l1_active_minus1");
				num_ref_idx_l1_active = ReadUvlc() + 1;
			}
		}
		Comment("RIR: ref_pic_list_reordering_flag");
		ref_pic_list_reordering_flag0 = ReadFlag();
		if (ref_pic_list_reordering_flag0)
		{
			int reordering_of_pic_nums_idc;
			int tt = 10;
			do {
				reordering_of_pic_nums_idc = ReadUvlc();
				int va = 0;
				if (reordering_of_pic_nums_idc != 3)
					va = ReadUvlc();
				reo0.push_back(makePair(reordering_of_pic_nums_idc, va+1));
				ASSERT(tt--);
			} while (reordering_of_pic_nums_idc != 3);
		}
		if (sp.SliceType==1 || sp.SliceType==6) // B, EB
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			ref_pic_list_reordering_flag1 = ReadFlag();
			int tt = 5;
			if (ref_pic_list_reordering_flag1)
			{
				int reordering_of_pic_nums_idc;
				do {
					reordering_of_pic_nums_idc = ReadUvlc();
					int va = 0;
					if (reordering_of_pic_nums_idc != 3)
						va = ReadUvlc();
					reo1.push_back(makePair(reordering_of_pic_nums_idc, va+1));
					ASSERT(tt--);
				} while (reordering_of_pic_nums_idc != 3);
			}
		}
	}

	if(pps.weighted_pred_flag && sp.SliceType==0 || pps.weighted_bipred_idc==1 && sp.SliceType==1)
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

	if (nal_ref_idc)
	{
		if (nal_unit_type==5 || sp.IDR)
		{
			Comment("DRPM: no_output_of_prior_pics_flag");
			int no_output_of_prior_pics_flag = ReadFlag();
			//printf("\t\tno_output_of_prior_pics_flag = %d\n", no_output_of_prior_pics_flag);
			Comment("DRPM: long_term_reference_flag");
			long_term_reference_flag = ReadFlag();
			//printf("\t\tlong_term_reference_flag = %d\n", long_term_reference_flag);
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
							if (slidingWindow.GetSliceParameters(ref_frames[sp.viewId][x], sp.viewId)->frameNum == fn)
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

	if (pps.cabac && sp.SliceType!=2)
	{
		Comment("SH: cabac_init_idc");
		sp.cabac_init_idc = ReadUvlc();
	}
	else
		sp.cabac_init_idc = 0;

	Comment("SH: slice_qp_delta");
	sp.QP_Delta = ReadSvlc();
	if (pps.deblock_filter_present)
	{
		int disable_deblocking_filter_idc = ReadUvlc();
		if (disable_deblocking_filter_idc!=1)
		{
			int slice_alpha_c0_offset_div2 = ReadSvlc();
			int slice_beta_offset_div2 = ReadSvlc();
		}
	}

	ArrayList<Pair<int, int> > list0, list1;
	if (sp.SliceType == 2)
	{
		; // Nothing
	}
	else if (sp.SliceType == 1)
	{
		for (int i = 0; i < ref_frames[sp.viewId].size(); ++i)
			list0.push_back(makePair(ref_frames[sp.viewId][i], sp.viewId));
		list1 = list0;
		for (int i = list0.size() - 1; i >= 0; --i)
		{
			for (int j = 0; j < i; ++j)
				if (list0[j].first>=sp.timeId && list0[j+1].first<sp.timeId ||
					list0[j+1].first<sp.timeId && list0[j+1].first>list0[j].first ||
					list0[j+1].first>=sp.timeId && list0[j+1].first<list0[j].first)
				{
					Pair<int, int> tmp = list0[j]; list0[j] = list0[j+1]; list0[j+1] = tmp;
				}
		}
		for (int i = list0.size() - 1; i >= 0; --i)
		{
			for (int j = 0; j < i; ++j)
				if (list1[j].first<=sp.timeId && list1[j+1].first>sp.timeId ||
					list1[j+1].first<=sp.timeId && list1[j+1].first>list1[j].first ||
					list1[j+1].first>sp.timeId && list1[j+1].first<list1[j].first)
				{
					Pair<int, int> tmp = list1[j]; list1[j] = list1[j+1]; list1[j+1] = tmp;
				}
		}
	}
	else if (sp.SliceType == 0)
	{
		for (int i = 0; i < ref_frames[sp.viewId].size(); ++i)
			list0.push_back(makePair(ref_frames[sp.viewId][i], sp.viewId));
		for (int i = list0.size() - 1; i >= 0; --i)
			for (int j = 0; j < i; ++j)
			{
				if (slidingWindow.GetSliceParameters(list0[j].first, list0[j].second)->frameNum <
					slidingWindow.GetSliceParameters(list0[j+1].first, list0[j+1].second)->frameNum)
				{
					Pair<int, int> tmp = list0[j]; list0[j] = list0[j+1]; list0[j+1] = tmp;
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
			list0.push_back(makePair(sp.timeId, sps.anchorRefsList0[sp.viewId][i]));
		for (int i = 0; i < sps.anchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(makePair(sp.timeId, sps.anchorRefsList1[sp.viewId][i]));
	}
	else
	{
		for (int i = 0; i < sps.nonAnchorRefsList0[sp.viewId].size(); ++i)
			list0.push_back(makePair(sp.timeId, sps.nonAnchorRefsList0[sp.viewId][i]));
		for (int i = 0; i < sps.nonAnchorRefsList1[sp.viewId].size(); ++i)
			list1.push_back(makePair(sp.timeId, sps.nonAnchorRefsList1[sp.viewId][i]));
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
				int ft = -1;
				for (int x = 0; x < ref_frames[sp.viewId].size(); ++x)
					if (slidingWindow.GetSliceParameters(ref_frames[sp.viewId][x], sp.viewId)->frameNum == uiIdentifier)
					{
						ft = ref_frames[sp.viewId][x];
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
							list0.erase(x);
							break;
						}
						list0.insert(uiIndex, makePair(ft, sp.viewId));
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
				for (int x = 0; x < list0.size(); ++x)
					if (list0[x].first == sp.timeId && list0[x].second == targetViewId)
					{   
						list0.erase(x);
						break;
					}  
					list0.insert(uiIndex, makePair(sp.timeId, targetViewId));

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
				int ft = -1;
				for (int x = 0; x < ref_frames[sp.viewId].size(); ++x)
					if (slidingWindow.GetSliceParameters(ref_frames[sp.viewId][x], sp.viewId)->frameNum == uiIdentifier)
					{
						ft = ref_frames[sp.viewId][x];
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
							list1.erase(x);
							break;
						}
						list1.insert(uiIndex, makePair(ft, sp.viewId));
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
				for (int x = 0; x < list1.size(); ++x)
					if (list1[x].first == sp.timeId && list1[x].second == targetViewId)
					{   
						list1.erase(x);
						break;
					}   
					list1.insert(uiIndex, makePair(sp.timeId, targetViewId));

					uiIndex++;   
			} // inter-view    
		} // while   
	}

	while (list0.size() > num_ref_idx_l0_active)
		list0.pop_back();
	while (list1.size() > num_ref_idx_l1_active)
		list1.pop_back();

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
				ref_frames[sp.viewId].push_back(sp.timeId);
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
				ref_frames[sp.viewId].push_back(sp.timeId);
			}
		}
	}
	else // 8.2.5.2
	{
		// TODO :
	}

	while (!(slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId)))
		Sleep(10);

	//printf("REF Count 0 = %d\n", list0.size());
	//for (int i = 0; i < list0.size(); ++i)
	//{
	//	printf("(%d,%d),", list0[i].first, list0[i].second);
	//}
	//printf("\n");
	//printf("REF Count 1 = %d\n", list1.size());
	//for (int i = 0; i < list1.size(); ++i)
	//{
	//	printf("(%d,%d),", list1[i].first, list1[i].second);
	//}
	//printf("\n");
	CodingPictInfo &info = *slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId);
	info.Ref_Count[0] = list0.size();
	for (int i = 0; i < list0.size(); ++i)
	{
		info.Ref_Id[0][i][0] = list0[i].first;
		info.Ref_Id[0][i][1] = list0[i].second;
		CodingPictInfo *ref = slidingWindow.GetCodingPictInfo(list0[i].first, list0[i].second);
		info.ref_lumaStart[0][i] = ref->lumaStart;
		info.ref_chromaUStart[0][i] = ref->chromaUStart;
		info.ref_chromaVStart[0][i] = ref->chromaVStart;
		info.ref_refVec[0][i] = ref->refVec;
	}
	info.Ref_Count[1] = list1.size();
	for (int i = 0; i < list1.size(); ++i)
	{
		info.Ref_Id[1][i][0] = list1[i].first;
		info.Ref_Id[1][i][1] = list1[i].second;
		CodingPictInfo *ref = slidingWindow.GetCodingPictInfo(list1[i].first, list1[i].second);
		info.ref_lumaStart[1][i] = ref->lumaStart;
		info.ref_chromaUStart[1][i] = ref->chromaUStart;
		info.ref_chromaVStart[1][i] = ref->chromaVStart;
		info.ref_refVec[1][i] = ref->refVec;
	}

	return (bs.tail - bs.start) * 32 + bs.pos;
}
