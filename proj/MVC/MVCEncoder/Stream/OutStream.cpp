#include "Stream/OutStream.h"
#include "Codec/Consts4Standard.h"
#include "Configure.h"
#include "SlidingWindow.h"
#include "CodecInfo.h"

int32_t OutStream::seq_p_id = 0;
int32_t OutStream::pic_p_id = 0;

OutStream::OutStream()
{
	buf = new uint8_t[50000];
	BitstreamInit(&bs, buf, 50000);
}

OutStream::~OutStream()
{
	delete[] buf;
}

void OutStream::Clear()
{
	BitstreamInit(&bs, buf, 50000);
}

uint8_t* OutStream::Finallize(int &len)
{
	BitstreamFlush(&bs);
	len = (int32_t)(bs.tail-bs.start)*4 + (bs.pos + 7) / 8;
	return buf;
}

void OutStream::Comment(char *str)
{
//	printf("%20s  ", str);
}

void OutStream::WriteCode(uint32_t code, uint32_t len)
{
	eg_write_direct(&bs, code, len);
//	printf("Code %08lX, %d\n", code, len);
}

void OutStream::WriteFlag(uint32_t code)
{
	eg_write_direct1(&bs, code);
//	printf("Flag %d\n", code);
}

void OutStream::WriteUvlc(uint32_t uvlc)
{
	eg_write_ue(&bs, uvlc);
//	printf("Uvlc %d\n", uvlc);
}

void OutStream::WriteSvlc(int32_t svlc)
{
	eg_write_se(&bs, svlc);
//	printf("Svlc %d\n", svlc);
}

void OutStream::PadByte()
{
	BitstreamPadOne(&bs);
}

int32_t OutStream::WriteSequenceParameterSet(SequenceParametersSet &sps)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_SPS, 5);

	Comment("SPS: profile_idc");
	WriteCode(HIGH_PROFILE, 8);
//	WriteCode(getProfileIdc(), 8);
	Comment("SPS: constrained_set0_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set1_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set2_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set3_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set4_flag");
	WriteFlag(false);
	Comment("SPS: reserved_zero_3bits");
	WriteCode(0, 3);
	Comment("SPS: level_idc");
	WriteCode(21, 8);
	//	WriteCode(getLevelIdc(), 8);
	Comment("SPS: seq_parameter_set_id");
	WriteUvlc(seq_p_id++);
	// getSeqParameterSetId()

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

	Comment("SPS: log2_max_frame_num_minus4");
	WriteUvlc(sps.log2nFrames - 4);

	Comment("SPS: pic_order_cnt_type");
	uint32_t picOrderCntType = 0; // getPicOrderCntType()
	WriteUvlc(picOrderCntType);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	WriteUvlc(sps.log2nFrames - 4); // ?
	// WriteUvlc(getLog2MaxPixOrderCntLsb()-4);

	Comment("SPS: num_ref_frames");
	WriteUvlc(sps.numRefFrames);
	Comment("SPS: required_frame_num_update_behaviour_flag");
	WriteFlag(1); // getRequiredFrameNumupdateBehaviourFlag()

	Comment("SPS: pic_width_in_mbs_minus_1");
	WriteUvlc((sps.width>>4) - 1);
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	WriteUvlc((sps.height>>4) - 1);
	Comment("SPS: frame_mbs_only_flag");
	WriteFlag(true);
	Comment("SPS: direct_8x8_inference_flag");
	WriteFlag(true); // getDirect8x8InferenceFlag()
	Comment("SPS: frame_cropping_flag");
	WriteFlag(false);
	Comment("SPS: vui_parameters_present_flag");
	WriteFlag(false);
	PadByte();
	return seq_p_id - 1;
}













int32_t OutStream::WriteSubsetSequenceParameterSet(SequenceParametersSet &sps)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_SUBSET_SPS, 5);

	Comment("SPS: profile_idc");
	WriteCode(MULTI_VIEW_PROFILE, 8);
//	WriteCode(getProfileIdc(), 8);
	Comment("SPS: constrained_set0_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set1_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set2_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set3_flag");
	WriteFlag(false);
	Comment("SPS: constrained_set4_flag");
	WriteFlag(false);
	Comment("SPS: reserved_zero_3bits");
	WriteCode(0, 3);
	Comment("SPS: level_idc");
	WriteCode(21, 8);
	//	WriteCode(getLevelIdc(), 8);
	Comment("SPS: seq_parameter_set_id");
	WriteUvlc(seq_p_id++);
	// getSeqParameterSetId()

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

	WriteUvlc(sps.log2nFrames - 4);

	Comment("SPS: pic_order_cnt_type");
	WriteUvlc(0);
	Comment("SPS: log2_max_pic_order_cnt_lsb_minus4");
	WriteUvlc(sps.log2nFrames - 4); // ?

	Comment("SPS: num_ref_frames");
	WriteUvlc(sps.numRefFrames);
	Comment("SPS: required_frame_num_update_behaviour_flag");
	WriteFlag(1); // getRequiredFrameNumupdateBehaviourFlag()

	Comment("SPS: pic_width_in_mbs_minus_1");
	WriteUvlc((sps.width>>4) - 1);
	Comment("SPS: pic_height_in_mbs_units_minus_1");
	WriteUvlc((sps.height>>4) - 1);
	Comment("SPS: frame_mbs_only_flag");
	WriteFlag(true);
	Comment("SPS: direct_8x8_inference_flag");
	WriteFlag(true); // getDirect8x8InferenceFlag()
	Comment("SPS: frame_cropping_flag");
	WriteFlag(false);
	Comment("SPS: vui_parameters_present_flag");
	WriteFlag(false);

	Comment("SUBSET SPS: bit_equal_to_one");
	WriteFlag(true);

	// seq_parameter_set_mvc_extension()
	Comment("SPS: num_views_minus_1");
	WriteUvlc(sps.viewCount - 1);

	for (int i = 0; i < sps.viewCount; ++i)
	{
		Comment("SPS: view_id[i]");
		WriteUvlc(sps.viewCodingOrder[i]); // SpsMVC->m_uiViewCodingOrder[i]
	}

	for (int i = 1; i < sps.viewCount; ++i)
	{
		int coding = sps.viewCodingOrder[i];
		int nAnchorRefs = sps.anchorRefsList0[coding].size();
		Comment("SPS: num_anchor_refs_l0[i]");
		WriteUvlc(nAnchorRefs);
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: anchor_ref_l0[i][j]");
			WriteUvlc(sps.anchorRefsList0[coding][j]);
		}

		nAnchorRefs = sps.anchorRefsList1[coding].size();
		Comment("SPS: num_anchor_refs_l1[i]");
		WriteUvlc(nAnchorRefs);
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: anchor_ref_l1[i][j]");
			WriteUvlc(sps.anchorRefsList1[coding][j]);
		}
	}

	for (int i = 1; i < sps.viewCount; ++i)
	{
		int coding = sps.viewCodingOrder[i];
		int nAnchorRefs = sps.nonAnchorRefsList0[coding].size();
		Comment("SPS: num_non_anchor_refs_l0[i]");
		WriteUvlc(nAnchorRefs);
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: non_anchor_ref_l0[i][j]");
			WriteUvlc(sps.nonAnchorRefsList0[coding][j]);
		}

		nAnchorRefs = sps.nonAnchorRefsList1[coding].size();
		Comment("SPS: num_non_anchor_refs_l1[i]");
		WriteUvlc(nAnchorRefs);
		for (int j = 0; j < nAnchorRefs; ++j)
		{
			Comment("SPS: non_anchor_ref_l1[i][j]");
			WriteUvlc(sps.nonAnchorRefsList1[coding][j]);
		}
	}

	Comment("SPS: num_level_values_signalled");
	WriteUvlc(0);
	// WriteUvlc((UInt &)SpsMVC->m_num_level_values_signalled);

	WriteCode(0, 8);
	WriteUvlc(0);
	WriteCode(0, 3);
	WriteUvlc(0);
	WriteUvlc(0);
	WriteUvlc(0);

	Comment("SUBSET SPS: mvc_vui_parameters_present_flag");
	WriteFlag(false);
	Comment("SUBSET SPS: Additional_extension2_flag");
	WriteFlag(false);
	WriteCode(0, 8);
	PadByte();

	return seq_p_id - 1;
}

int32_t OutStream::WritePictureParameterSet(const PictureParametersSet &pps, int32_t seq_parameter_set_id)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_PPS, 5);

	Comment("PPS: pic_parameter_set_id");
	WriteUvlc(pic_p_id++);
	// getPicParameterSetId()
	Comment("PPS: seq_parameter_set_id");
	WriteUvlc(seq_parameter_set_id);
	// getSeqParameterSetId()
	Comment("PPS: entropy_coding_mode_flag");
	WriteFlag(0); // CAVLC, 1 CABAC
	Comment("PPS: pic_order_present_flag");
	WriteFlag(true);
	// getPicOrderPresentFlag()
	Comment("PPS: num_slice_groups_minus1");
	WriteUvlc(0);

	Comment("PPS: num_ref_idx_l0_active_minus1");
	WriteUvlc(pps.num_ref_idx_l0_active - 1);
	Comment("PPS: num_ref_idx_l1_active_minus1");
	WriteUvlc(pps.num_ref_idx_l1_active - 1);
	Comment("PPS: weighted_pred_flag");
	WriteFlag(0);
	Comment("PPS: weighted_bipred_idc");
	WriteCode(0, 2);
	Comment("PPS: pic_init_qp_minus26");
	WriteSvlc(pps.baseQP - 26);
	Comment("PPS: pic_init_qs_minus26");
	WriteSvlc(0);
	Comment("PPS: chroma_qp_index_offset");
	WriteSvlc(0);
	// getChomaQpIndexOffset()
	Comment("PPS: deblocking_filter_parameters_present_flag");
	WriteFlag(0);
	// getDeblockingFilterParametersPresentFlag()
	Comment("PPS: constrained_intra_pred_flag");
	WriteFlag(0);
	// getConstrainedIntraPredFlag()
	Comment("PPS: redundant_pic_cnt_present_flag");
	WriteFlag(0);
	// getRedundantPicCntPresentFlag()

	Comment("PPS: transform_8x8_mode_flag");
	WriteFlag(1);
	Comment("PPS: pic_scaling_matrix_present_flag");
	WriteFlag(0);
	Comment("PPS: second_chroma_qp_index_offset");
	WriteSvlc(0);
	PadByte();

	return pic_p_id - 1;
}

void OutStream::WriteSlicePrefix(const SliceParameters &sp)
{
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	WriteCode(3, 2);
	Comment("NALU HEADER: nal_unit_type");
	WriteCode(NAL_UNIT_CODED_SLICE_PREFIX, 5);
	Comment("NALU HEADER: svc_mvc_flag");
	WriteFlag(0);
	//WriteCode(this->getSvcMvcFlag(), 1);
	Comment("NALU HEADER: non_idr_flag");
	WriteFlag(!sp.IDR);
	Comment("NALU HEADER: priority_id");
	WriteCode(sp.priority_id, 6);
	Comment("NALU HEADER: view_id");
	WriteCode(sp.viewId, 10);
	Comment("NALU HEADER: temporal_id");
	WriteCode(0, 3); // ?
	Comment("NALU HEADER: anchor_pic_flag");
	WriteFlag(sp.isAnchor); // ?
	Comment("NALU HEADER: inter_view_flag");
	WriteFlag(1); // ?
	Comment("NALU HEADER: reserved_zero_one_bit");
	WriteFlag(0);
	PadByte();
}

int OutStream::WriteSliceHeader(const SliceParameters &sp, SlidingWindow &slidingWindow)
{
	SequenceParametersSet &sps = CodecInfo::GetInstance().sps;
	PictureParametersSet &pps = CodecInfo::GetInstance().pps;
	Comment("NALU HEADER: forbidden_zero_bit");
	WriteFlag(0);
	Comment("NALU HEADER: nal_ref_idc");
	int nal_ref_idc = 3;
	if (!sp.isRef)
		nal_ref_idc = 0;
	WriteCode(nal_ref_idc,   2);
	Comment("NALU HEADER: nal_unit_type");
	int nal_unit_type;
	if (sp.viewId == sps.viewCodingOrder[0])
	{
		if (sp.IDR)
			nal_unit_type = NAL_UNIT_CODED_SLICE_IDR;
		else
			nal_unit_type = NAL_UNIT_CODED_SLICE;
	}
	else
		nal_unit_type = NAL_UNIT_CODED_SLICE_SCALABLE;
	WriteCode(nal_unit_type, 5);
	if (nal_unit_type == 20)
	{
		Comment("NALU HEADER: svc_mvc_flag");
		WriteFlag(0);
		Comment("NALU HEADER: non_idr_flag");
		WriteFlag(!sp.IDR);
		Comment("NALU HEADER: priority_id");
		WriteCode(sp.priority_id, 6);
		Comment("NALU HEADER: view_id");
		WriteCode(sp.viewId, 10);
		Comment("NALU HEADER: temporal_id");
		WriteCode(0, 3);
		Comment("NALU HEADER: anchor_pic_flag");
		WriteFlag(sp.isAnchor);
		Comment("NALU HEADER: inter_view_flag");
		WriteFlag(1);
		Comment("NALU HEADER: reserved_zero_one_bit");
		WriteFlag(0);
	}
	Comment("SH: first_mb_in_slice");
	WriteUvlc(0);
	Comment("SH: slice_type");
	WriteUvlc(sp.SliceType);
	Comment("SH: pic_parameter_set_id");
	WriteUvlc(nal_unit_type==20);
	Comment("SH: frame_num");
	WriteCode(sp.frameNum & sps.nFramesM1, sps.log2nFrames);
	if( sp.IDR && nal_unit_type==NAL_UNIT_CODED_SLICE_IDR )
	{
		Comment("SH: idr_pic_id");
		WriteUvlc(0);
	}
	Comment("SH: pic_order_cnt_lsb");
	WriteCode(sp.timeId & sps.nFramesM1, sps.log2nFrames);
	Comment("SH: delta_pic_order_cnt_bottom");
	WriteSvlc(0);

	if (sp.SliceType==1 || sp.SliceType==6) // B, EB
	{
		Comment("SH: direct_spatial_mv_pred_flag");
		WriteFlag(1);
	}


	CodingPictInfo &cpi = *slidingWindow.GetCodingPictInfo(sp.timeId, sp.viewId);
	if (sp.SliceType==0 || sp.SliceType==1 || sp.SliceType==5 || sp.SliceType==6)
	{
		int tref0 = cpi.Ref_Count[0], tref1 = cpi.Ref_Count[1];
		Comment("SH: num_ref_idx_active_override_flag");
		if (tref0!=pps.num_ref_idx_l0_active || (sp.SliceType==1 || sp.SliceType==6) && tref1!=pps.num_ref_idx_l1_active)
		{
			WriteFlag(1);
			Comment("SH: num_ref_idx_l0_active_minus1");
			WriteUvlc(tref0 - 1);
			if (sp.SliceType==1 || sp.SliceType==6)
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
		for (int i = 0; i < cpi.Ref_Count[0]; ++i)
		{
			if (cpi.Ref_Id[0][i][0]==sp.timeId)
			{
				int vId = 0;
				if (sp.isAnchor)
				{
					for (int j = 0; j < sps.anchorRefsList0[sp.viewId].size(); ++j)
						if (sps.anchorRefsList0[sp.viewId][j] == cpi.Ref_Id[0][i][1])
						{
							vId = j;
							break;
						}
				}
				else
				{
					for (int j = 0; j < sps.nonAnchorRefsList0[sp.viewId].size(); ++j)
						if (sps.nonAnchorRefsList0[sp.viewId][j] == cpi.Ref_Id[0][i][1])
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
				SliceParameters &spr = *slidingWindow.GetSliceParameters(cpi.Ref_Id[0][i][0], cpi.Ref_Id[0][i][1]);
				if (spr.frameNum>picNum)
				{
					WriteUvlc(RPLR_POS);
					WriteUvlc(spr.frameNum - picNum - 1);
				}
				else
				{
					WriteUvlc(RPLR_NEG);
					WriteUvlc(picNum - spr.frameNum - 1);
				}
				picNum = spr.frameNum;
			}
		}
		WriteUvlc(RPLR_END);
		if (sp.SliceType==1 || sp.SliceType==6) // B, EB
		{
			Comment("RIR: ref_pic_list_reordering_flag");
			WriteFlag(1);
			int picNum = sp.frameNum;
			int viewIdx = -1;
			for (int i = 0; i < cpi.Ref_Count[1]; ++i)
			{
				if (cpi.Ref_Id[1][i][0]==sp.timeId)
				{
					int vId = 0;
					if (sp.isAnchor)
					{
						for (int j = 0; j < sps.anchorRefsList1[sp.viewId].size(); ++j)
							if (sps.anchorRefsList1[sp.viewId][j] == cpi.Ref_Id[1][i][1])
							{
								vId = j;
								break;
							}
					}
					else
					{
						for (int j = 0; j < sps.nonAnchorRefsList1[sp.viewId].size(); ++j)
							if (sps.nonAnchorRefsList1[sp.viewId][j] == cpi.Ref_Id[1][i][1])
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
					SliceParameters &spr = *slidingWindow.GetSliceParameters(cpi.Ref_Id[1][i][0], cpi.Ref_Id[1][i][1]);
					if (spr.frameNum>picNum)
					{
						WriteUvlc(RPLR_POS);
						WriteUvlc(spr.frameNum - picNum - 1);
					}
					else
					{
						WriteUvlc(RPLR_NEG);
						WriteUvlc(picNum - spr.frameNum - 1);
					}
					picNum = spr.frameNum;
				}
			}
			WriteUvlc(RPLR_END);
		}
	}


	if (nal_ref_idc)
	{
		if (nal_unit_type==5 || sp.IDR)
		{
			Comment("DRPM: no_output_of_prior_pics_flag");
			WriteFlag(1);
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

	Comment("SH: slice_qp_delta");
	WriteSvlc(0);
	return (bs.tail - bs.start) * 32 + bs.pos;
}

