/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Create
 * 
 ************************************************************************/

#include "PSlice.h"
#include "PBMacroblockEncode.h"
#include "Codec/CAVLC.h"
#include "PBCAVLC.h"
#include "PBPredict.h"
#include "Codec/Consts4Standard.h"
#include "Arch/Arch.h"
#include "Codec/ConfigureSlave.h"
#include "Codec/vlcPred.h"

// For PSNR
#include "math.h"

/*
void require_data_decode(CodingPictInfo *cpi, macroblockdata *list, PBPredictInfo *plist, macroblockInfo *mlist, int32_t cur_r, int32_t cur_c)
{
	int32_t i = 0, j;
	int32_t MB_height = cpi->Size_Height >> 4;
	int32_t MB_width = cpi->Size_Width >> 4;

	plist[i].haveHalf = false;

	for (int ls = 0; ls < 2; ++ls)
	{
		for (int ri = 0; ri < cpi->Ref_Count[ls]; ++ri)
		{

			uint8_t *rLuma = cpi->ref_lumaStart[ls][ri] + cpi->Size_Width * cur_r * 16 + cur_c * 16,
				*rChromaU = cpi->ref_chromaUStart[ls][ri] + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8,
				*rChromaV = cpi->ref_chromaVStart[ls][ri] + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8;
			int hsize_w = 8, hsize_h = 8;
			int vleft = PB_PRED_RANGE/2, vtop = PB_PRED_RANGE/2;

			int mleft = min(vleft, 8 * cur_c);
			rLuma -= mleft * 2;
			rChromaU -= mleft;
			rChromaV -= mleft;
			hsize_w += mleft;
			vleft -= mleft;

			int rig = min(PB_PRED_RANGE/2, (MB_width-1-cur_c) * 8);;
			hsize_w += rig;

			int mtop = min(vtop, 8 * cur_r);
			rLuma -= cpi->Size_Width * 2 * mtop;;
			rChromaU -= cpi->Size_Width / 2 * mtop;
			rChromaV -= cpi->Size_Width / 2 * mtop;
			hsize_h += mtop;
			vtop -= mtop;

			hsize_h += min(PB_PRED_RANGE/2, (MB_height-1-cur_r) * 8);

			for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
			{
				GetAndWait(&plist[i].data_y0[ls][ri][j][vleft<<1], (unsigned int)rLuma, hsize_w<<1);
				rLuma += cpi->Size_Width;

			}
			for (j = vtop; j < vtop+hsize_h; ++j)
			{
				GetAndWait(&plist[i].data_u0[ls][ri][j][vleft], (unsigned int)rChromaU, hsize_w);
				GetAndWait(&plist[i].data_v0[ls][ri][j][vleft], (unsigned int)rChromaV, hsize_w);
				rChromaU += cpi->Size_Width / 2;
				rChromaV += cpi->Size_Width / 2;
			}
			if (vleft)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][j][vleft*2], vleft*2);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][j][vleft], vleft);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][j][vleft], vleft);
			}
			if (rig < PB_PRED_RANGE/2)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(&plist[i].data_y0[ls][ri][j][(vleft+hsize_w)*2], plist[i].data_y0[ls][ri][j][(vleft+hsize_w)*2-1], (PB_PRED_RANGE/2-rig)*2);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_u0[ls][ri][j][vleft+hsize_w], plist[i].data_u0[ls][ri][j][vleft+hsize_w-1], PB_PRED_RANGE/2-rig);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_v0[ls][ri][j][vleft+hsize_w], plist[i].data_v0[ls][ri][j][vleft+hsize_w-1], PB_PRED_RANGE/2-rig);
			}
			for (j = 0; j < vtop<<1; ++j)
				memcpy(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][vtop*2], sizeof(plist[i].data_y0[ls][ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][vtop], sizeof(plist[i].data_u0[ls][ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][vtop], sizeof(plist[i].data_v0[ls][ri][j]));

			for (j = (vtop+hsize_h)<<1; j < 16 + PB_PRED_RANGE * 2; ++j)
				memcpy(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][(vtop+hsize_h)*2-1], sizeof(plist[i].data_y0[ls][ri][j]));
			for (j = vtop+hsize_h; j < 8 + PB_PRED_RANGE; ++j)
				memcpy(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][vtop+hsize_h-1], sizeof(plist[i].data_u0[ls][ri][j]));
			for (j = vtop+hsize_h; j < 8 + PB_PRED_RANGE; ++j)
				memcpy(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][vtop+hsize_h-1], sizeof(plist[i].data_v0[ls][ri][j]));
		}
	}
}
*/

void require_data_encode(CodingPictInfo *cpi, macroblockdata *list, PBPredictInfo *plist, macroblockInfo *mlist, int32_t cur_r, int32_t cur_c)
{
	int32_t i = 0, j;
	int32_t MB_height = cpi->Size_Height >> 4;
	int32_t MB_width = cpi->Size_Width >> 4;
	memset(&mlist[i], 0, sizeof(mlist[i]));
	mlist[i].x = cur_r;
	mlist[i].y = cur_c;

	uint8_t *luma = cpi->lumaStart + cpi->Size_Width * cur_r * 16 + cur_c * 16,
		*chromaU = cpi->chromaUStart + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8,
		*chromaV = cpi->chromaVStart + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8;
	for (j = 0; j < 16; ++j)
	{
		GetAndWait(&list[i].y[j][0], (unsigned int)luma, sizeof(list[i].y[j]));
		luma += cpi->Size_Width;
	}
	for (j = 0; j < 8; ++j)
	{
		GetAndWait(list[i].uv[0][j], chromaU, sizeof(list[i].uv[0][j]));
		GetAndWait(list[i].uv[1][j], chromaV, sizeof(list[i].uv[1][j]));
		chromaU += cpi->Size_Width / 2;
		chromaV += cpi->Size_Width / 2;
	}

	//plist[i].haveHalf = false;

	for (int ls = 0; ls < 2; ++ls)
	{
		for (int ri = 0; ri < cpi->Ref_Count[ls]; ++ri)
		{
			plist[i].myr = cur_r;
			plist[i].myc = cur_c;
			plist[i].stride = cpi->Size_Width;
			plist[i].sizew = cpi->Size_Width;
			plist[i].sizeh = cpi->Size_Height;
			plist[i].pdata_y0[ls][ri] = cpi->ref_lumaStart[ls][ri];
			plist[i].pdata_u0[ls][ri] = cpi->ref_chromaUStart[ls][ri];
			plist[i].pdata_v0[ls][ri] = cpi->ref_chromaVStart[ls][ri];


			/*
			// Begin to be replaced
			uint8_t *rLuma = cpi->ref_lumaStart[0][ri] + cpi->Size_Width * cur_r * 16 + cur_c * 16,
				*rChromaU = cpi->ref_chromaUStart[0][ri] + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8,
				*rChromaV = cpi->ref_chromaVStart[0][ri] + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8;

			int hsize_w = 8, hsize_h = 8;
			int vleft = PB_PRED_RANGE/2, vtop = PB_PRED_RANGE/2;

			int mleft = min(vleft, 8 * cur_c);
			rLuma -= mleft * 2;
			rChromaU -= mleft;
			rChromaV -= mleft;
			hsize_w += mleft;
			vleft -= mleft;

			int rig = min(PB_PRED_RANGE/2, (MB_width-1-cur_c) * 8);;
			hsize_w += rig;

			int mtop = min(vtop, 8 * cur_r);
			rLuma -= cpi->Size_Width * 2 * mtop;;
			rChromaU -= cpi->Size_Width / 2 * mtop;
			rChromaV -= cpi->Size_Width / 2 * mtop;
			hsize_h += mtop;
			vtop -= mtop;

			hsize_h += min(PB_PRED_RANGE/2, (MB_height-1-cur_r) * 8);

			for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
			{
				GetAndWait(&plist[i].data_y0[ls][ri][j][vleft<<1], (unsigned int)rLuma, hsize_w<<1);
				rLuma += cpi->Size_Width;

			}
			for (j = vtop; j < vtop+hsize_h; ++j)
			{
				GetAndWait(&plist[i].data_u0[ls][ri][j][vleft], (unsigned int)rChromaU, hsize_w);
				GetAndWait(&plist[i].data_v0[ls][ri][j][vleft], (unsigned int)rChromaV, hsize_w);
				rChromaU += cpi->Size_Width / 2;
				rChromaV += cpi->Size_Width / 2;
			}
			if (vleft)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][j][vleft*2], vleft*2);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][j][vleft], vleft);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][j][vleft], vleft);
			}
			if (rig < PB_PRED_RANGE/2)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(&plist[i].data_y0[ls][ri][j][(vleft+hsize_w)*2], plist[i].data_y0[ls][ri][j][(vleft+hsize_w)*2-1], (PB_PRED_RANGE/2-rig)*2);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_u0[ls][ri][j][vleft+hsize_w], plist[i].data_u0[ls][ri][j][vleft+hsize_w-1], PB_PRED_RANGE/2-rig);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_v0[ls][ri][j][vleft+hsize_w], plist[i].data_v0[ls][ri][j][vleft+hsize_w-1], PB_PRED_RANGE/2-rig);
			}
			for (j = 0; j < vtop<<1; ++j)
				memcpy(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][vtop*2], sizeof(plist[i].data_y0[ls][ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][vtop], sizeof(plist[i].data_u0[ls][ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][vtop], sizeof(plist[i].data_v0[ls][ri][j]));

			for (j = (vtop+hsize_h)<<1; j < 16 + PB_PRED_RANGE * 2; ++j)
				memcpy(plist[i].data_y0[ls][ri][j], plist[i].data_y0[ls][ri][(vtop+hsize_h)*2-1], sizeof(plist[i].data_y0[ls][ri][j]));
			for (j = vtop+hsize_h; j < 8 + PB_PRED_RANGE; ++j)
				memcpy(plist[i].data_u0[ls][ri][j], plist[i].data_u0[ls][ri][vtop+hsize_h-1], sizeof(plist[i].data_u0[ls][ri][j]));
			for (j = vtop+hsize_h; j < 8 + PB_PRED_RANGE; ++j)
				memcpy(plist[i].data_v0[ls][ri][j], plist[i].data_v0[ls][ri][vtop+hsize_h-1], sizeof(plist[i].data_v0[ls][ri][j]));
			// End to be replaced
			*/

			/*
			uint8_t hasLeft = 0;
			uint8_t hasTop = 0;
			int hsize_w = 8, hsize_h = 8;
			int vleft = PB_PRED_RANGE/2, vtop = PB_PRED_RANGE/2;
			if (cur_c)
			{
				hasLeft = 1;
				rLuma -= 16;
				rChromaU -= 8;
				rChromaV -= 8;
				hsize_w += 8;
				vleft -= 8;
			}
			if (cur_c<MB_width-1)
			{
				hsize_w += 8;
			}

			if (cur_r)
			{
				hasTop = 1;
				rLuma -= cpi->Size_Width * 16;
				rChromaU -= cpi->Size_Width / 2 * 8;
				rChromaV -= cpi->Size_Width / 2 * 8;
				hsize_h += 8;
				vtop -= 8;
			}
			if (cur_r<MB_height-1)
			{
				hsize_h += 8;
			}

			for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
			{
				GetAndWait(&plist[i].data_y0[ri][j][vleft<<1], (unsigned int)rLuma, hsize_w<<1);
				rLuma += cpi->Size_Width;

			}
			for (j = vtop; j < vtop+hsize_h; ++j)
			{
				GetAndWait(&plist[i].data_u0[ri][j][vleft], (unsigned int)rChromaU, hsize_w);
				GetAndWait(&plist[i].data_v0[ri][j][vleft], (unsigned int)rChromaV, hsize_w);
				rChromaU += cpi->Size_Width / 2;
				rChromaV += cpi->Size_Width / 2;
			}
			if (!cur_c)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(plist[i].data_y0[ri][j], plist[i].data_y0[ri][j][16], 16);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_u0[ri][j], plist[i].data_u0[ri][j][8], 8);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(plist[i].data_v0[ri][j], plist[i].data_v0[ri][j][8], 8);
			}
			if (cur_c>=MB_height-1)
			{
				for (j = vtop<<1; j < ((vtop+hsize_h)<<1); ++j)
					memset(&plist[i].data_y0[ri][j][32], plist[i].data_y0[ri][j][31], 16);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_u0[ri][j][16], plist[i].data_u0[ri][j][15], 8);
				for (j = vtop; j < vtop+hsize_h; ++j)
					memset(&plist[i].data_v0[ri][j][16], plist[i].data_v0[ri][j][15], 8);
			}
			for (j = 0; j < vtop<<1; ++j)
				memcpy(plist[i].data_y0[ri][j], plist[i].data_y0[ri][16], sizeof(plist[i].data_y0[ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_u0[ri][j], plist[i].data_u0[ri][8], sizeof(plist[i].data_u0[ri][j]));
			for (j = 0; j < vtop; ++j)
				memcpy(plist[i].data_v0[ri][j], plist[i].data_v0[ri][8], sizeof(plist[i].data_v0[ri][j]));

			for (j = (vtop+hsize_h)<<1; j < 48; ++j)
				memcpy(plist[i].data_y0[ri][j], plist[i].data_y0[ri][31], sizeof(plist[i].data_y0[ri][j]));
			for (j = vtop+hsize_h; j < 24; ++j)
				memcpy(plist[i].data_u0[ri][j], plist[i].data_u0[ri][15], sizeof(plist[i].data_u0[ri][j]));
			for (j = vtop+hsize_h; j < 24; ++j)
				memcpy(plist[i].data_v0[ri][j], plist[i].data_v0[ri][15], sizeof(plist[i].data_v0[ri][j]));
			*/
		}
	}

}

void save_data_pb(CodingPictInfo *cpi, macroblockdata *dlist, int32_t cur_r, int32_t cur_c)
{
	int32_t j;
	int32_t MB_width = cpi->Size_Width >> 4;
	uint8_t *luma = cpi->lumaStart + cpi->Size_Width * cur_r * 16 + cur_c * 16,
		*chromaU = cpi->chromaUStart + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8,
		*chromaV = cpi->chromaVStart + cpi->Size_Width / 2 * cur_r * 8 + cur_c * 8;
	for (j = 0; j < 16; ++j)
	{
		PutAndWait(dlist->y[j], luma, sizeof(dlist->y[j]));
		luma += cpi->Size_Width;
	}
	for (j = 0; j < 8; ++j)
	{
		PutAndWait(dlist->uv[0][j], chromaU, sizeof(dlist->uv[0][j]));
		PutAndWait(dlist->uv[1][j], chromaV, sizeof(dlist->uv[1][j]));
		chromaU += cpi->Size_Width / 2;
		chromaV += cpi->Size_Width / 2;
	}
}

int16_t median(int16_t a, int16_t b, int16_t c)
{
	if (a>=b && a<=c)
		return a;
	if (b>=a && b<=c)
		return b;
	return c;
}

void process_data_encode(CodingPictInfo *cpi, macroblockdata *dlist, PBPredictInfo *plist, macroblockInfo *mlist)
{
	DEBUGBLK(29, ("process_data_encode"));

	// ²Î¿¼ºê¿é(16x16)
	macroblockdata pred0;

	memset(&pred0, 128, sizeof(pred0));


	mlist->mode_i16x16 = 0;
	memset(mlist->mode_i4x4, 0, sizeof(mlist->mode_i4x4));
	mlist->mode_uv = 0;

	mlist->qp_delta = 0; // Delta QP not supported.

	mlist->cbp_y = 15;
	mlist->cbp_uv = 2;
	macroblockPBPredictionGetBest(cpi, plist, dlist, mlist, &pred0);
	macroblockInterEncode(dlist, &pred0, cpi->baseQP, cpi->baseQP, mlist);

}

int32_t is_data_required_PB(int32_t idx)
{
	return 1;
}

int32_t is_data_saved_PB(int32_t idx)
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

int macroblock_processor_PB(void *param)
{
	macroblock_info *mi = (macroblock_info *)param;
	macroblockdata datalist;
	PBPredictInfo predlist;
	int row = mi->row;
	int MBwidth = mi->cpi->Size_Width >> 4;
	for (int i = 0; i < MBwidth; ++i)
	{
		while (mi->allow<=i)
			Sleep(0);
		require_data_encode(mi->cpi, &datalist, &predlist, mi->mblist+i, row, i);
		process_data_encode(mi->cpi, &datalist, &predlist, mi->mblist+i);
		save_data_pb(mi->cpi, &datalist, row, i);
		mi->next->allow = i - 1;
	}
	mi->next->allow = MBwidth;
	return 0;
}

void PSliceEncode(CodingPictInfo *cpi, Bitstream *bs)
{
	bool isParallel = cpi->MP * MB_E > 1.0;
	int MBheight = cpi->Size_Height >> 4,
		MBwidth = cpi->Size_Width >> 4;
	macroblockInfo *mlist = new macroblockInfo[MBheight * MBwidth];
	macroblock_info *mis = new macroblock_info[/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP + 1];
	for (int i = 0; i <= /*ConfigureSlave::PARALLEL_SIZE*/cpi->MP; ++i)
	{
		mis[i].cpi = cpi;
		mis[i].allow = MBwidth;
		mis[i].next = mis+i+1;
	}
	mis[/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP].next = mis;
	int p = 0, p1 = p + 1, p2 = p1 + 1;
	if (/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP==1)
		p2 = 0;
	for (int i = 0; i < MBheight; ++i)
	{
		while (mis[p2].allow != MBwidth)
			Sleep(1);
		mis[p1].allow = 0;
		mis[p].row = i;
		mis[p].mblist = mlist + i * MBwidth;
		if (/*ConfigureSlave::NO_PARALLEL*/!isParallel)
			macroblock_processor_PB(mis+p);
		else
			MVCCreateMasterThread(macroblock_processor_PB, mis+p);
		p = p1; p1 = p2;
		if (p2==/*ConfigureSlave::PARALLEL_SIZE*/cpi->MP)
			p2 = 0;
		else
			++p2;

	}

	while (mis[p].allow != MBwidth)
		Sleep(1);

	int16_t		refVecs[1024][2];
	memset(refVecs, 0, sizeof(refVecs));

	cavlc_ref_list ref_list;
	cavlc_ref *topRef = new cavlc_ref[MBwidth * 4 + 5];
	memset(topRef, 0, sizeof(cavlc_ref) * (MBwidth * 4 + 5));
	for (int i = 0; i < MBwidth * 4 + 5; ++i)
	{
		topRef[i].coeff = 0x40;
		topRef[i].coeffC = 0x40;
		topRef[i].imode = 0x40;
		topRef[i].mv_x[0] = 0x4000;
		topRef[i].mv_y[0] = 0x4000;
		topRef[i].mv_x[1] = 0x4000;
		topRef[i].mv_y[1] = 0x4000;
	}
	ref_list.left = new cavlc_ref[4];

	int skip = 0;
	int i = 0;
	for (int r = 0; r < MBheight; ++r)
	{
		memset(ref_list.left, 0, sizeof(cavlc_ref) * 4);
		for (int j = 0; j < 4; ++j)
		{
			ref_list.left[j].coeff = 0x40;
			ref_list.left[j].coeffC = 0x40;
			ref_list.left[j].mv_x[0] = 0x4000;
			ref_list.left[j].mv_y[0] = 0x4000;
			ref_list.left[j].mv_x[1] = 0x4000;
			ref_list.left[j].mv_y[1] = 0x4000;
		}
		ref_list.topleft = ref_list.left[0];
		ref_list.top = topRef + 1;
		for (int c = 0; c < MBwidth; ++c)
		{
			cavlc_ref newTL = ref_list.top[3];
			refVector refV;
			if (mlist[i].cbp_y==0 && mlist[i].cbp_uv==0)
			{
				mlist[i].qp_delta = 0;
			}

			if (mlist[i].cbp_y==0 && mlist[i].cbp_uv==0 && 
				mlist[i].refs[0][0][0].refno==0 && 
				mlist[i].refs[0][0][1].refno == -1 &&
				mlist[i].mb_part == MB_16x16 &&
				(cpi->PictType==0 || mlist[i].mb_part2==B_L0_16x16))
			{
				copyVector(refV, ref_list.topleft);
				getRefXY(&ref_list, 0, 0, 4, refV, true);
				if (mlist[i].refs[0][0][0].x == refV.x &&
					mlist[i].refs[0][0][0].y == refV.y)
					mlist[i].is_skip = 1;
			}

			//static int id = 0;
			if (mlist[i].is_skip)
			{
				//printf("mb_type(%d) = %s\n", id++, "Skip");
				++skip;
				for (int i = 0; i< 4; ++i)
				{
					ref_list.left[i].coeff = 0;
					ref_list.left[i].coeffC = 0;
					ref_list.top[i].coeff = 0;
					ref_list.top[i].coeffC = 0;
					ref_list.left[i].mv_x[0] = refV.x;
					ref_list.left[i].mv_y[0] = refV.y;
					ref_list.top[i].mv_x[0] = refV.x;
					ref_list.top[i].mv_y[0] = refV.y;
				}
			}
			else
			{
				macroblock_write_cavlc_skip(bs, skip);
				skip = 0;
				//printf("mb_type(%d) = %d\n", id++, mlist[i].mb_part);
				if (cpi->PictType == 0)
					macroblock_write_cavlc_P_slice(bs, &mlist[i], cpi, &ref_list);
				else
					macroblock_write_cavlc_B_slice(bs, &mlist[i], cpi, &ref_list);
			}
			++i;
			ref_list.top += 4;
			ref_list.topleft = newTL;
		}
	}
	if (skip)
		macroblock_write_cavlc_skip(bs, skip);

	/*
	for (int i = 0; i < MBheight * MBwidth; ++i)
	{
		if (cpi->viewId == 2 && cpi->timeId == 0)
		{
			for (int j = 0; j < 16; ++j)
				printf("(%d,%d), ", 
				mlist[i].refs[j/4][j%4][0].x,
				mlist[i].refs[j/4][j%4][0].y);
			printf("\n");
			for (int j = 0; j < 16; ++j)
				printf("(%d,%d), ", 
				mlist[i].refs[j/4][j%4][1].x,
				mlist[i].refs[j/4][j%4][1].y);
			printf("\n");
			printf("\n");

		}
	}
	*/
	/*
	if (cpi->viewId == 2)
		exit(0);
	*/

	delete []mis;
	delete []mlist;
}

