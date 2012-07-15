/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Modified from T264
 * 
 ************************************************************************/

#include "Codec/CAVLC.h"
#include "PBCAVLC.h"
#include "Codec/Bitstream.h"
#include "Codec/vlc.h"
#include "Common.h"
#include "Codec/Consts4Standard.h"
#include "PictureInfo.h"
#include "../IFrame/ICAVLC.h"

#include <stdio.h>

static const uint8_t intra4x4_cbp_to_golomb[48]=
{
  3, 29, 30, 17, 31, 18, 37,  8, 32, 38, 19,  9, 20, 10, 11,  2,
 16, 33, 34, 21, 35, 22, 39,  4, 36, 40, 23,  5, 24,  6,  7,  1,
 41, 42, 43, 25, 44, 26, 46, 12, 45, 47, 27, 13, 28, 14, 15,  0
};
static const uint8_t inter_cbp_to_golomb[48]=
{
  0,  2,  3,  7,  4,  8, 17, 13,  5, 18,  9, 14, 10, 15, 16, 11,
  1, 32, 33, 36, 34, 37, 44, 40, 35, 45, 38, 41, 39, 42, 43, 19,
  6, 24, 25, 20, 26, 21, 46, 28, 27, 47, 22, 29, 23, 30, 31, 12
};

/*
static const int8_t luma_index[] = 
{
	0, 1, 4, 5,
	2, 3, 6, 7,
	8, 9, 12, 13,
	10, 11, 14, 15
};
*/

static const int8_t luma_inverse_x[] = 
{
	0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3
};

static const int8_t luma_inverse_y[] = 
{
	0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};

void macroblock_read_cavlc_PB_y(Bitstream *bs, pmacroblockInfo info, cavlc_ref_list *ref_list)
{
	int32_t cbp;

	cbp = inter_eg_to_cbp[eg_read_ue(bs)];
	info->cbp_y = cbp % 16;
	info->cbp_uv = cbp / 16;

	info->transform8x8 = 0;
	bool ok = true;
	for (int i = 0; i < 4; ++i)
	{
		if (info->submb_part[i] > MB_8x8 && info->submb_part[i]!=MB_8x8_DIR)
			ok = false;
	}
	//if (pps.transform8x8Flag)
	if (ok && info->cbp_y)
	{
		info->transform8x8 = eg_read_direct1(bs);
	}
	if (cbp > 0)
		info->qp_delta = eg_read_se(bs);
	else
		info->qp_delta = 0;
	info->coded44 |= 0;
	for(int i = 0 ; i < 4 ; i ++)
		if (info->cbp_y & (1 << i))
		{
			for (int j = 0; j < 4; ++j)
			{
				int idx = i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2;
				int nC = get_luma_pred(ref_list, idx);
				nC = block_residual_read_cavlc(bs, idx, info->dct_y_z[idx], 16, nC);
				set_luma_pred(ref_list, idx, nC);
				if (nC || info->transform8x8)
				{
					info->coded44 |= (1<<(idx));
				}
			}
		}
		else
		{
			for (int j = 0; j < 4; ++j)
			{
				int idx = i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2;
				memset(info->dct_y_z[idx], 0, 16);
				set_luma_pred(ref_list, idx, 0);
			}
		}
}


int id = 0;

void macroblock_read_cavlc_P_slice(Bitstream *bs, pmacroblockInfo info, CodingPictInfo *cpi, cavlc_ref_list *ref_list)
{
	for (int i = 0; i < 4; ++i)
	{
		ref_list->tops[i] = ref_list->top[i];
		ref_list->lefts[i] = ref_list->left[i];
	}
	refVector predV;
	info->mbtype = 0;
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			info->refs[i][j][0].refno = -1;
			info->refs[i][j][1].refno = -1;
		}
	int32_t mb_type;
	if (info->is_skip < 0)
		info->is_skip = eg_read_ue(bs);
	if (info->is_skip--)
	{
		info->mb_part = MB_16x16;
		info->mb_part2 = B_L0_16x16;
		copyVector(predV, ref_list->topleft);
		getRefXY_SKIP(ref_list, 0, 0, 0, 4, predV, true);
		if (predV.refno < 0)
		{
			predV.x = predV.y = predV.refno = 0;
		}
		info->refs[0][0][0].refno = 0;
		info->refs[0][0][0].x = predV.x;
		info->refs[0][0][0].y = predV.y;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				info->refs[i][j][0] = info->refs[0][0][0];
				info->refs[i][j][1] = info->refs[0][0][1];
			}
		for (int i = 0; i< 4; ++i)
		{
			ref_list->left[i].coeff = 0;
			ref_list->left[i].coeffC = 0;
			ref_list->top[i].coeff = 0;
			ref_list->top[i].coeffC = 0;
			ref_list->left[i].mv_x[0] = predV.x;
			ref_list->left[i].mv_y[0] = predV.y;
			ref_list->left[i].refid[0] = predV.refno;
			ref_list->top[i].mv_x[0] = predV.x;
			ref_list->top[i].mv_y[0] = predV.y;
			ref_list->top[i].refid[0] = predV.refno;
		}
		for (int j = 0; j < 4; ++j)
			ref_list->left[j].imode = 0x02;
		for (int j = 0; j < 4; ++j)
			ref_list->top[j].imode = 0x02;
		return ;
	}
	mb_type = eg_read_ue(bs);
	refVector vec1;

	info->mb_part = mb_type;
	mb_type = -1;   /* ugly way: prevent break to i slice code */
	vec1.refno = 0;
	int refId[4] = {0};

	switch (info->mb_part) 
	{
	case MB_16x16:
		if (cpi->Ref_Count[0]>1)
		{
			if (cpi->Ref_Count[0]==2)
				refId[0] = 1 - eg_read_direct1(bs);
			else
				refId[0] = eg_read_ue(bs);
		}
		else
			refId[0] = 0;
		copyVector(predV, ref_list->topleft);
		getRefXYP(ref_list, 0, 0, 4, predV, true, 0, refId[0]);
		info->refs[0][0][0].refno = refId[0];
		info->refs[0][0][0].y = predV.y + eg_read_se(bs);
		info->refs[0][0][0].x = predV.x + eg_read_se(bs);
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				info->refs[i][j][0] = info->refs[0][0][0];
				info->refs[i][j][1] = info->refs[0][0][1];
			}
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 2, 2, info->refs[0][0][0]);
		setRefXY(ref_list, 3, 3, info->refs[0][0][0]);
		break;
	case MB_16x8:
		for (int i = 0; i < 2; ++i)
		{
			if (cpi->Ref_Count[0]>1)
			{
				if (cpi->Ref_Count[0]==2)
					refId[i] = 1 - eg_read_direct1(bs);
				else
					refId[i] = eg_read_ue(bs);
			}
			else
				refId[i] = 0;
		}
		copyVector(predV, ref_list->topleft);
		getRefXYP(ref_list, 0, 0, 4, predV, true, 2, refId[0]);
		info->refs[0][0][0].refno = refId[0];
		info->refs[0][0][0].y = predV.y + eg_read_se(bs);
		info->refs[0][0][0].x = predV.x + eg_read_se(bs);
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 0, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 2, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 3, info->refs[0][0][0]);

		copyVector(predV, ref_list->lefts[1]);
		getRefXYP(ref_list, 2, 0, 4, predV, false, 1, refId[1]);
		info->refs[2][0][0].refno = refId[1];
		info->refs[2][0][0].y = predV.y + eg_read_se(bs);
		info->refs[2][0][0].x = predV.x + eg_read_se(bs);
		setRefXY(ref_list, 2, 0, info->refs[2][0][0]);
		setRefXY(ref_list, 2, 1, info->refs[2][0][0]);
		setRefXY(ref_list, 3, 2, info->refs[2][0][0]);
		setRefXY(ref_list, 3, 3, info->refs[2][0][0]);
		info->refs[1][0][0] = info->refs[0][0][0];
		info->refs[1][0][1] = info->refs[0][0][1];
		info->refs[3][0][0] = info->refs[2][0][0];
		info->refs[3][0][1] = info->refs[2][0][1];
		for (int i = 0; i < 4; ++i)
			for (int j = 1; j < 4; ++j)
			{
				info->refs[i][j][0] = info->refs[i][0][0];
				info->refs[i][j][1] = info->refs[i][0][1];
			}
		break;
	case MB_8x16:
		for (int i = 0; i < 2; ++i)
		{
			if (cpi->Ref_Count[0]>1)
			{
				if (cpi->Ref_Count[0]==2)
					refId[i] = 1 - eg_read_direct1(bs);
				else
					refId[i] = eg_read_ue(bs);
			}
			else
				refId[i] = 0;
		}
		copyVector(predV, ref_list->topleft);
		getRefXYP(ref_list, 0, 0, 2, predV, true, 1, refId[0]);
		info->refs[0][0][0].refno = refId[0];
		info->refs[0][0][0].y = predV.y + eg_read_se(bs);
		info->refs[0][0][0].x = predV.x + eg_read_se(bs);
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 2, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 3, 1, info->refs[0][0][0]);

		copyVector(predV, ref_list->tops[1]);
		getRefXYP(ref_list, 0, 2, 4, predV, true, 3, refId[1]);
		info->refs[0][2][0].refno = refId[1];
		info->refs[0][2][0].y = predV.y + eg_read_se(bs);
		info->refs[0][2][0].x = predV.x + eg_read_se(bs);
		setRefXY(ref_list, 0, 2, info->refs[0][2][0]);
		setRefXY(ref_list, 1, 2, info->refs[0][2][0]);
		setRefXY(ref_list, 2, 3, info->refs[0][2][0]);
		setRefXY(ref_list, 3, 3, info->refs[0][2][0]);
		info->refs[0][1][0] = info->refs[0][0][0];
		info->refs[0][1][1] = info->refs[0][0][1];
		info->refs[0][3][0] = info->refs[0][2][0];
		info->refs[0][3][1] = info->refs[0][2][1];
		for (int i = 1; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
			{
				info->refs[i][j][0] = info->refs[0][j][0];
				info->refs[i][j][1] = info->refs[0][j][1];
			}
		break;
	case MB_8x8:
	case MB_8x8ref0:
		for (int i = 0; i < 4; ++i)
		{
			int code = eg_read_ue(bs);
			switch (code)
			{
			case 0: info->submb_part[i] = MB_8x8; break;
			case 1: info->submb_part[i] = MB_8x4; break;
			case 2: info->submb_part[i] = MB_4x8; break;
			case 3: info->submb_part[i] = MB_4x4; break;
			default :
				assert(0);
			}
		}

		for (int i = 0; i < 4; ++i)
		{
			if (cpi->Ref_Count[0]>1 && info->mb_part!=MB_8x8ref0)
			{
				if (cpi->Ref_Count[0]==2)
					refId[i] = 1 - eg_read_direct1(bs);
				else
					refId[i] = eg_read_ue(bs);
			}
			else
				refId[i] = 0;
		}
		info->mb_part = MB_8x8;
		//info->refs[0][0][0].refno = 0;
		//info->refs[0][2][0].refno = 0;
		//info->refs[2][0][0].refno = 0;
		//info->refs[2][2][0].refno = 0;
		for (int i = 0; i < 4; ++i)
		{
			int xx = i / 2 * 2;
			int yy = i % 2 * 2;
			int st = i * 4;

			switch (i)
			{
			case 0: copyVector(predV, ref_list->topleft); break;
			case 1: copyVector(predV, ref_list->tops[1]); break;
			case 2: copyVector(predV, ref_list->lefts[1]); break;
			case 3: predV = info->refs[1][1][0]; break;
			default: break;
			}
			switch (info->submb_part[i])
			{
			case MB_8x8:
				getRefXYP(ref_list, xx, yy, yy+2, predV, i<3, 0, refId[i]);
				info->refs[xx][yy][0].refno = refId[i];
				info->refs[xx][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx][yy][0]);
				info->refs[xx+1][yy][0] = info->refs[xx][yy][0];
				info->refs[xx+1][yy][1] = info->refs[xx][yy][1];
				info->refs[xx][yy+1][0] = info->refs[xx][yy][0];
				info->refs[xx][yy+1][1] = info->refs[xx][yy][1];
				info->refs[xx+1][yy+1][0] = info->refs[xx][yy][0];
				info->refs[xx+1][yy+1][1] = info->refs[xx][yy][1];
				break;
			case MB_8x4:
				getRefXYP(ref_list, xx, yy, yy+2, predV, i<3, 0, refId[i]);
				info->refs[xx][yy][0].refno = refId[i];
				info->refs[xx][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx, yy+1, info->refs[xx][yy][0]);

				if (i&1)
					predV = info->refs[xx][yy-1][0];
				else
					copyVector(predV, ref_list->lefts[xx]);
				getRefXYP(ref_list, xx+1, yy, yy+2, predV, false, 0, refId[i]);
				info->refs[xx+1][yy][0].refno = refId[i];
				info->refs[xx+1][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx+1][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx+1, yy, info->refs[xx+1][yy][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx+1][yy][0]);

				//
				info->refs[xx][yy+1][0] = info->refs[xx][yy][0];
				info->refs[xx][yy+1][1] = info->refs[xx][yy][1];
				info->refs[xx+1][yy+1][0] = info->refs[xx+1][yy][0];
				info->refs[xx+1][yy+1][1] = info->refs[xx+1][yy][1];
				//
				break;
			case MB_4x8:
				getRefXYP(ref_list, xx, yy, yy+1, predV, true, 0, refId[i]);
				info->refs[xx][yy][0].refno = refId[i];
				info->refs[xx][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx+1, yy, info->refs[xx][yy][0]);

				if (i<2)
					copyVector(predV, ref_list->tops[yy]);
				else
					predV = info->refs[xx-1][yy][0];
				getRefXYP(ref_list, xx, yy+1, yy+2, predV, i<3, 0, refId[i]);
				info->refs[xx][yy+1][0].refno = refId[i];
				info->refs[xx][yy+1][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy+1][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy+1, info->refs[xx][yy+1][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx][yy+1][0]);
				//
				info->refs[xx+1][yy][0] = info->refs[xx][yy][0];
				info->refs[xx+1][yy][1] = info->refs[xx][yy][1];
				info->refs[xx+1][yy+1][0] = info->refs[xx][yy+1][0];
				info->refs[xx+1][yy+1][1] = info->refs[xx][yy+1][1];
				//
				break;
			case MB_4x4:

				getRefXYP(ref_list, xx, yy, yy+1, predV, true, 0, refId[i]);
				info->refs[xx][yy][0].refno = refId[i];
				info->refs[xx][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);

				if (i<2)
					copyVector(predV, ref_list->tops[yy]);
				else
					predV = info->refs[xx-1][yy][0];
				getRefXYP(ref_list, xx, yy+1, yy+2, predV, i<3, 0, refId[i]);
				info->refs[xx][yy+1][0].refno = refId[i];
				info->refs[xx][yy+1][0].y = predV.y + eg_read_se(bs);
				info->refs[xx][yy+1][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx, yy+1, info->refs[xx][yy+1][0]);

				if (i&1)
					predV = info->refs[xx][yy-1][0];
				else
					copyVector(predV, ref_list->lefts[xx]);
				getRefXYP(ref_list, xx+1, yy, yy+1, predV, true, 0, refId[i]);
				info->refs[xx+1][yy][0].refno = refId[i];
				info->refs[xx+1][yy][0].y = predV.y + eg_read_se(bs);
				info->refs[xx+1][yy][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx+1, yy, info->refs[xx+1][yy][0]);

				predV = info->refs[xx][yy][0];
				getRefXYP(ref_list, xx+1, yy+1, yy+2, predV, false, 0, refId[i]);
				info->refs[xx+1][yy+1][0].refno = refId[i];
				info->refs[xx+1][yy+1][0].y = predV.y + eg_read_se(bs);
				info->refs[xx+1][yy+1][0].x = predV.x + eg_read_se(bs);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx+1][yy+1][0]);

				break;
			}
		}
		// todo:
		break;
	default:
		info->mbtype = 2;
		info->mb_part -= 5;
		T264dec_mb_read_intra_cavlc(bs, info->mb_part, info, ref_list);
		for (int i = 0; i< 4; ++i)
		{
			ref_list->left[i].refid[0] = -1;
			ref_list->left[i].mv_x[0] = 0x2000;
			ref_list->left[i].mv_y[0] = 0x2000;
			ref_list->top[i].refid[0] = -1;
			ref_list->top[i].mv_x[0] = 0x2000;
			ref_list->top[i].mv_y[0] = 0x2000;
		}
		return;
	}

	for (int j = 0; j < 4; ++j)
		ref_list->left[j].imode = 0x02;
	for (int j = 0; j < 4; ++j)
		ref_list->top[j].imode = 0x02;

	macroblock_read_cavlc_PB_y(bs, info, ref_list);
	macroblock_read_cavlc_uv(bs, info, ref_list);

}

void calc_B_skip_refs(refVector *col_refs, pmacroblockInfo info, cavlc_ref_list *ref_list)
{
	refVector predV[2];
	refVector predV2[2];
	copyVector(predV2[0], ref_list->topleft);
	copyVector2(predV2[1], ref_list->topleft);


	for (int mb88 = 0; mb88 < 4; ++mb88)
	{
		int row = mb88 / 2 * 2, col = mb88 % 2 * 2;
		predV[0] = predV2[0];
		predV[1] = predV2[1];
		getRefXYDirectB(ref_list, col_refs, info, row, col, col + 2, predV);
		info->refs[row][col][0] = predV[0];
		info->refs[row][col][1] = predV[1];
		setRefXY(ref_list, 0, row+1, col+0, predV[0]);
		setRefXY(ref_list, 0, row+0, col+1, predV[0]);
		setRefXY(ref_list, 1, row+1, col+0, predV[1]);
		setRefXY(ref_list, 1, row+0, col+1, predV[1]);
	}

	for (int i = 0; i < 4; ++i)
	{
		int dx = (i / 2) * 2, dy = (i % 2) * 2;
		info->refs[dx][dy+1][0] = info->refs[dx][dy][0];
		info->refs[dx][dy+1][1] = info->refs[dx][dy][1];
		info->refs[dx+1][dy][0] = info->refs[dx][dy][0];
		info->refs[dx+1][dy][1] = info->refs[dx][dy][1];
		info->refs[dx+1][dy+1][0] = info->refs[dx][dy][0];
		info->refs[dx+1][dy+1][1] = info->refs[dx][dy][1];
	}
}

void macroblock_read_cavlc_B_slice(Bitstream *bs, refVector *col_refs, pmacroblockInfo info, CodingPictInfo *cpi, cavlc_ref_list *ref_list)
{
	for (int i = 0; i < 5; ++i)
	{
		ref_list->tops[i] = ref_list->top[i];
	}
	for (int i = 0; i < 4; ++i)
		ref_list->lefts[i] = ref_list->left[i];
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			info->refs[i][j][0].refno = -1;
			info->refs[i][j][1].refno = -1;
		}
	int32_t mb_type = 1;
	if (info->is_skip < 0)
		info->is_skip = eg_read_ue(bs);
//	static int id = 0;
	info->mbtype = 1;

	if (info->is_skip--)
	{
		info->mb_part = MB_16x16;
		info->mb_part2 = B_L0_16x16;
		calc_B_skip_refs(col_refs, info, ref_list);
		
		for (int i = 0; i< 4; ++i)
		{
			ref_list->top[i].imode = 2;
			ref_list->left[i].imode = 2;
			ref_list->left[i].coeff = 0;
			ref_list->left[i].coeffC = 0;
			ref_list->top[i].coeff = 0;
			ref_list->top[i].coeffC = 0;
			setRefXY(ref_list, 0, 3, i, info->refs[3][i][0]);
			setRefXY(ref_list, 1, 3, i, info->refs[3][i][1]);
			setRefXY(ref_list, 0, i, 3, info->refs[i][3][0]);
			setRefXY(ref_list, 1, i, 3, info->refs[i][3][1]);
		}
		return ;
	}

	refVector predV;
	mb_type = eg_read_ue(bs);

	refVector vecPred0, vecPred1;
	refVector vec1[2];	//save the mv info of second Partition
	vecPred0.refno = 0;	//default reference index
	vecPred1.refno = 0;
	vec1[0].refno = vec1[1].refno = 0;

	info->mb_part = mb_type;
	for (int i = 0; i < 4; ++i)
		info->submb_part[i] = MB_8x8;
	int l1 = 0, l2 = 0;
	uint32_t RN = 0x00000000;
	switch (mb_type) 
	{
	case 0:	//B_Direct_16x16
		{
			calc_B_skip_refs(col_refs, info, ref_list);
			for (int i = 0; i< 4; ++i)
			{
				setRefXY(ref_list, 0, 3, i, info->refs[3][i][0]);
				setRefXY(ref_list, 1, 3, i, info->refs[3][i][1]);
				setRefXY(ref_list, 0, i, 3, info->refs[i][3][0]);
				setRefXY(ref_list, 1, i, 3, info->refs[i][3][1]);
			}

		}

		info->mb_part = MB_16x16;
		info->mb_part2 = B_L0_16x16;
		RN = 0x00000000;
		// TODO:
		break;
	case 1:
		info->mb_part = MB_16x16;
		info->mb_part2 = B_L0_16x16;
		RN = 0x00000001;
		break;
	case 2:
		info->mb_part = MB_16x16;
		info->mb_part2 = B_L1_16x16;
		RN = 0x00010000;
		break;
	case 3:
		info->mb_part = MB_16x16;
		info->mb_part2 = B_Bi_16x16;
		RN = 0x00010001;
		break;
	case 4:	//B_L0_L0_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L0_L0_16x8;
		RN = 0x00000101;
		break;
	case 6: //B_L1_L1_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L1_L1_16x8;
		RN = 0x01010000;
		break;
	case 8://B_L0_L1_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L0_L1_16x8;
		RN = 0x01000001;
		break;
	case 10:	//B_L1_L0_16x8:
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L1_L0_16x8;
		RN = 0x00010100;
		break;
	case 12:	//B_L0_Bi_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L0_Bi_16x8;
		RN = 0x01000101;
		break;
	case 14://B_L1_Bi_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_L1_Bi_16x8;
		RN = 0x01010100;
		break;
	case 16:	//B_Bi_L0_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_Bi_L0_16x8;
		RN = 0x00010101;
		break;
	case 18://B_Bi_L1_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_Bi_L1_16x8;
		RN = 0x01010001;
		break;
	case 20:	//B_Bi_Bi_16x8
		info->mb_part = MB_16x8;
		info->mb_part2 = B_Bi_Bi_16x8;
		RN = 0x01010101;
		break;
	case 5:	//B_L0_L0_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L0_L0_8x16;
		RN = 0x00000011;
		break;
	case 7:	//B_L1_L1_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L1_L1_8x16;
		RN = 0x00110000;
		break;
	case 9:	//B_L0_L1_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L0_L1_8x16;
		RN = 0x00100001;
		break;
	case 11://B_L1_L0_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L1_L0_8x16;
		RN = 0x00010010;
		break;
	case 13://B_L0_Bi_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L0_Bi_8x16;
		RN = 0x00100011;
		break;
	case 15:	//B_L1_Bi_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_L1_Bi_8x16;
		RN = 0x00110010;
		break;
	case 17:	//B_Bi_L0_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_Bi_L0_8x16;
		RN = 0x00010011;
		break;
	case 19://B_Bi_L1_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_Bi_L1_8x16;
		RN = 0x00110001;
		break;
	case 21://B_Bi_Bi_8x16
		info->mb_part = MB_8x16;
		info->mb_part2 = B_Bi_Bi_8x16;
		RN = 0x00110011;
		break;


	case 22:	//B_8x8
		{
			RN = 0x00000000;
			info->mb_part = MB_8x8;
			for (int i = 0; i < 4; ++i)
			{
				int tmp = eg_read_ue(bs);
				info->submb_part[i] = MBTableMode[tmp];
				switch (info->submb_part[i])
				{
				case MB_8x8: RN |= (MBTableFwd[tmp]*0x01) << (i*4); break;
				case MB_8x4: RN |= (MBTableFwd[tmp]*0x05) << (i*4); break;
				case MB_4x8: RN |= (MBTableFwd[tmp]*0x03) << (i*4); break;
				case MB_4x4: RN |= (MBTableFwd[tmp]*0x0F) << (i*4); break;
				default : // MB_8x8_DIR
					;
				}
			}
		}
		// TODO:
		// todo:
		break;
	default:
		info->mbtype = 2;
		info->mb_part -= 23;
		T264dec_mb_read_intra_cavlc(bs, info->mb_part, info, ref_list);
		for (int i = 0; i< 4; ++i)
		{
			ref_list->left[i].refid[0] = -1;
			ref_list->left[i].mv_x[0] = 0x2000;
			ref_list->left[i].mv_y[0] = 0x2000;
			ref_list->top[i].refid[0] = -1;
			ref_list->top[i].mv_x[0] = 0x2000;
			ref_list->top[i].mv_y[0] = 0x2000;
			ref_list->left[i].refid[1] = -1;
			ref_list->left[i].mv_x[1] = 0x2000;
			ref_list->left[i].mv_y[1] = 0x2000;
			ref_list->top[i].refid[1] = -1;
			ref_list->top[i].mv_x[1] = 0x2000;
			ref_list->top[i].mv_y[1] = 0x2000;
		}
		return;
	}
	if (mb_type>0)
	{
		int rn = RN;
		for (int i = 0; i < 4; ++i)
		{
			if (rn & 0x0F)
			{
				int x = i / 2 * 2;
				int y = i % 2 * 2;
				if (cpi->Ref_Count[0]>1)
					info->refs[x][y][0].refno = eg_read_te(bs, cpi->Ref_Count[0]-1);
				else
					info->refs[x][y][0].refno =  0;
			}
			rn >>= 4;
		}
		for (int i = 0; i < 4; ++i)
		{
			if (rn & 0x0F)
			{
				int x = i / 2 * 2;
				int y = i % 2 * 2;
				if (cpi->Ref_Count[1]>1)
					info->refs[x][y][1].refno = eg_read_te(bs, cpi->Ref_Count[1]-1);
				else
					info->refs[x][y][1].refno = 0;
			}
			rn >>= 4;
		}
		rn = RN;
		int topp[5] = {0, 0, 0, 0, 0};
		refVector predV1[2];
		refVector predV2[2];
		copyVector(predV2[0], ref_list->topleft);
		copyVector2(predV2[1], ref_list->topleft);
		//getRefXY(ref_list, 0, 0, 0, 4, predV, true, 0, info->refs[0][0][0].refno);
		//predV0 = predV;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int p2;
				int x = i / 2 * 2 + j / 2;
				int y = i % 2 * 2 + j % 2;
				p2 = x + ((info->mb_part==MB_16x16 || info->mb_part==MB_8x16)?4:
					(((info->submb_part[i]==MB_8x8)||(info->submb_part[i]==MB_8x8_DIR)||(info->submb_part[i]==MB_4x8))?2:1));
				//			p2 = x+(((info->submb_part[i]==MB_8x8)||(info->submb_part[i]==MB_8x8_DIR)||(info->submb_part[i]==MB_4x8))?2:1);
				int z = y + ((info->mb_part==MB_16x16 || info->mb_part==MB_16x8)?4:
					(((info->submb_part[i]==MB_8x8)||(info->submb_part[i]==MB_8x8_DIR)||(info->submb_part[i]==MB_8x4))?2:1));

				if (j==0 && (i&1) && (info->mb_part==MB_16x8 || info->mb_part==MB_16x16))
				{
					info->refs[x][y][0] = info->refs[x][0][0];
				}
				else if (j==0 && (i&2) && (info->mb_part==MB_8x16 || info->mb_part==MB_16x16))
				{
					info->refs[x][y][0] = info->refs[0][y][0];
				}

				int dy = 0, dx = 0;
				if (rn&1)
				{
					dy = eg_read_se(bs);
					dx = eg_read_se(bs);
				}
				if ((rn&1) || (j==0 && info->submb_part[i]==MB_8x8_DIR))
				{
					if (info->submb_part[i]==MB_8x8_DIR)
					{
						int row = i / 2 * 2, col = i % 2 * 2;
						predV1[0] = predV2[0];
						predV1[1] = predV2[1];
						getRefXYDirectB(ref_list, col_refs, info, row, col, col + 2, predV1);
						info->refs[row][col][0] = predV1[0];
						info->refs[row][col][1] = predV1[1];
						predV = predV1[0];
					}
					else
					{
						if (x==0)
						{
							if (y==0)
								copyVector(predV, ref_list->topleft);
							else
								copyVector(predV, ref_list->tops[y-1]);
						}
						else
						{
							if (y==0)
								copyVector(predV, ref_list->lefts[x-1]);
							else
							{
								int px = x - 1, py = y - 1;
								predV = info->refs[px][py][0];
							}
						}

						int type = 0;
						if (info->mb_part==MB_16x8)
						{
							if (i==0)
								type = 2;
							else
								type = 1;
						}
						else if (info->mb_part==MB_8x16)
						{
							if (i==0)
								type = 1;
							else
								type = 3;
						}
						getRefXY(ref_list, 0, x, y, z, predV, topp[z]==x, type, info->refs[x&14][y&14][0].refno);

					}

					if (predV.refno==-1 && (rn&1))
					{

						predV.refno = 0;
						predV.x = 0;
						predV.y = 0;
					}
					if (info->refs[x][y][0].refno < 0)
						info->refs[x][y][0].refno = predV.refno;
					if (predV.refno >= 0)
					{
						info->refs[x][y][0].y = dy + predV.y;
						info->refs[x][y][0].x = dx + predV.x;
					}
					else
					{
						info->refs[x][y][0].y = predV.y;
						info->refs[x][y][0].x = predV.x;
					}
				}
				if (topp[y] <= x)
				{
					for (int r1 = x; r1 < p2; ++r1)
					{
						for (int r2 = y; r2 < z; ++r2)
						{
							topp[r2] = p2;
							info->refs[r1][r2][0] = info->refs[x][y][0];
							setRefXY(ref_list, 0, r1, r2, info->refs[x][y][0]);
						}
					}
				}
				rn >>= 1;
			}
		}
		topp[0] = topp[1] = topp[2] = topp[3] = topp[4] = 0;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				int x = i / 2 * 2 + j / 2;
				int y = i % 2 * 2 + j % 2;
				int p2;
				p2 = x + ((info->mb_part==MB_16x16 || info->mb_part==MB_8x16)?4:
					(((info->submb_part[i]==MB_8x8)||(info->submb_part[i]==MB_8x8_DIR)||(info->submb_part[i]==MB_4x8))?2:1));
				int z = y + ((info->mb_part==MB_16x16 || info->mb_part==MB_16x8)?4:
					(((info->submb_part[i]==MB_8x8)||(info->submb_part[i]==MB_8x8_DIR)||(info->submb_part[i]==MB_8x4))?2:1));

				if (j==0 && (i&1) && (info->mb_part==MB_16x8 || info->mb_part==MB_16x16))
				{
					info->refs[x][y][1] = info->refs[x][0][1];
				}
				else if (j==0 && (i&2) && (info->mb_part==MB_8x16 || info->mb_part==MB_16x16))
				{
					info->refs[x][y][1] = info->refs[0][y][1];
				}

				int dy = 0, dx = 0;
				if (rn&1)
				{
					dy = eg_read_se(bs);
					dx = eg_read_se(bs);
				}
				if ((rn&1) || (j==0 && info->submb_part[i]==MB_8x8_DIR))
				{
					if (info->submb_part[i]==MB_8x8_DIR)
					{
						int row = i / 2 * 2, col = i % 2 * 2;
						predV = info->refs[row][col][1];
					}
					else// if (j==0)
					{
						if (x==0)
						{
							if (y==0)
								copyVector2(predV, ref_list->topleft);
							else
								copyVector2(predV, ref_list->tops[y-1]);
						}
						else
						{
							if (y==0)
							{
								//printf("");
								copyVector2(predV, ref_list->lefts[x-1]);
							}
							else
							{
								int px = x - 1, py = y - 1;
								predV = info->refs[px][py][1];
							}
						}

						int type = 0;
						if (info->mb_part==MB_16x8)
						{
							if (i==0)
								type = 2;
							else
								type = 1;
						}
						else if (info->mb_part==MB_8x16)
						{
							if (i==0)
								type = 1;
							else
								type = 3;
						}
						getRefXY(ref_list, 1, x, y, z, predV, topp[z]==x, type, info->refs[x&14][y&14][1].refno);


					}

					if (predV.refno==-1 && (rn&1))
					{

						predV.refno = 0;
						predV.x = 0;
						predV.y = 0;
					}
					if (info->refs[x][y][1].refno < 0)
						info->refs[x][y][1].refno = predV.refno;
					if (predV.refno >= 0)
					{
						info->refs[x][y][1].y = dy + predV.y;
						info->refs[x][y][1].x = dx + predV.x;
					}
					else
					{
						info->refs[x][y][1].y = predV.y;
						info->refs[x][y][1].x = predV.x;
					}
				}
				if (topp[y] <= x)
				{
					for (int r1 = x; r1 < p2; ++r1)
					{
						for (int r2 = y; r2 < z; ++r2)
						{
							topp[r2] = p2;
							info->refs[r1][r2][1] = info->refs[x][y][1];
							setRefXY(ref_list, 1, r1, r2, info->refs[x][y][1]);
						}
					}
				}
				rn >>= 1;
			}
		}
	}

	for (int i = 0; i < 4; ++i)
	{
		ref_list->top[i].imode = 2;
		ref_list->left[i].imode = 2;
	}
	macroblock_read_cavlc_PB_y(bs, info, ref_list);
	macroblock_read_cavlc_uv(bs, info, ref_list);
}
