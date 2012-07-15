/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Create
 * 
 ************************************************************************/

#include "PBCAVLC.h"
#include "Codec/CAVLC.h"
#include "Codec/Consts4Standard.h"

static const uint8_t inter_cbp_to_golomb[48]=
{
	0,  2,  3,  7,  4,  8, 17, 13,  5, 18,  9, 14, 10, 15, 16, 11,
	1, 32, 33, 36, 34, 37, 44, 40, 35, 45, 38, 41, 39, 42, 43, 19,
	6, 24, 25, 20, 26, 21, 46, 28, 27, 47, 22, 29, 23, 30, 31, 12
};

void macroblock_write_cavlc_PB_y(Bitstream *bs, pmacroblockInfo info, cavlc_ref_list *ref_list)
{
	eg_write_ue(bs, inter_cbp_to_golomb[(info->cbp_uv << 4)| info->cbp_y]);
	bool ok = true;
	for (int i = 0; i < 4; ++i)
	{
		if (info->submb_part[i] > MB_8x8)
			ok = false;
	}
	//if (pps.transform8x8Flag)
	if (ok && info->cbp_y)
	{
		eg_write_direct1(bs, 0);
//		printf("transform8x8 = %d\n", 0);
	}
	if (info->cbp_y > 0 || info->cbp_uv > 0)
		eg_write_se(bs, info->qp_delta);	/* 0 = no change on qp */
//	printf("cbp_y = %d\n", info->cbp_y);
	for(int i = 0 ; i < 4 ; i ++)
		if (info->cbp_y & (1 << i))
		{
			for (int j = 0; j < 4; ++j)
			{
				int idx = i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2;
				int nC = get_luma_pred(ref_list, idx);
//				printf("%d ", nC);
				nC = block_residual_write_cavlc(bs, i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2, info->dct_y_z[i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2], 16, nC);
				set_luma_pred(ref_list, idx, nC);
			}
		}
		else
		{
			for (int j = 0; j < 4; ++j)
			{
				int idx = i/2 * 8 + i % 2 * 2 + j / 2 * 4 + j % 2;
				set_luma_pred(ref_list, idx, 0);
			}
		}
//		printf("\n");
}

void macroblock_write_cavlc_skip(Bitstream *bs, int skip)
{
	eg_write_ue(bs, skip);
}

void macroblock_write_cavlc_P_slice(Bitstream *bs, pmacroblockInfo info, CodingPictInfo *cpi, cavlc_ref_list *ref_list)
{
	int skip = 5;
	int i;

	if (info->mb_part==MB_8x8 || info->mb_part==MB_8x8ref0)
	{
		info->mb_part = MB_8x8ref0;
		/*		// PSlice, only reference 1 frame
		if (info->ref[0].refno==0 &&
		info->ref[4].refno==0 &&
		info->ref[8].refno==0 &&
		info->ref[12].refno==0)
		info->mb_part = MB_8x8ref0;
		else
		info->mb_part = MB_8x8;
		*/
	}
	eg_write_ue(bs, info->mb_part);  /* skip run */
	skip = 0;

	//refVector vec;
	refVector predV;
	switch (info->mb_part) 
	{
	case MB_16x16:
		copyVector(predV, ref_list->topleft);
		getRefXY(ref_list, 0, 0, 4, predV, true);
		eg_write_se(bs, info->refs[0][0][0].y - predV.y);
		eg_write_se(bs, info->refs[0][0][0].x - predV.x);
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 2, 2, info->refs[0][0][0]);
		setRefXY(ref_list, 3, 3, info->refs[0][0][0]);
		break;
	case MB_16x8:
		copyVector(predV, ref_list->topleft);
		getRefXY(ref_list, 0, 0, 4, predV, true, 2);
		eg_write_se(bs, info->refs[0][0][0].y - predV.y);
		eg_write_se(bs, info->refs[0][0][0].x - predV.x);
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 0, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 2, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 3, info->refs[0][0][0]);
		copyVector(predV, ref_list->lefts[1]);
		getRefXY(ref_list, 2, 0, 4, predV, false, 1);
		eg_write_se(bs, info->refs[2][0][0].y - predV.y);
		eg_write_se(bs, info->refs[2][0][0].x - predV.x);
		setRefXY(ref_list, 2, 0, info->refs[2][0][0]);
		setRefXY(ref_list, 2, 1, info->refs[2][0][0]);
		setRefXY(ref_list, 3, 2, info->refs[2][0][0]);
		setRefXY(ref_list, 3, 3, info->refs[2][0][0]);
		break;
	case MB_8x16:
		copyVector(predV, ref_list->topleft);
		getRefXY(ref_list, 0, 0, 4, predV, true, 1);
		eg_write_se(bs, info->refs[0][0][0].y - predV.y);
		eg_write_se(bs, info->refs[0][0][0].x - predV.x);
		setRefXY(ref_list, 0, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 1, 0, info->refs[0][0][0]);
		setRefXY(ref_list, 2, 1, info->refs[0][0][0]);
		setRefXY(ref_list, 3, 1, info->refs[0][0][0]);
		copyVector(predV, ref_list->tops[1]);
		getRefXY(ref_list, 0, 2, 4, predV, true, 3);
		eg_write_se(bs, info->refs[0][2][0].y - predV.y);
		eg_write_se(bs, info->refs[0][2][0].x - predV.x);
		setRefXY(ref_list, 0, 0, info->refs[0][2][0]);
		setRefXY(ref_list, 1, 0, info->refs[0][2][0]);
		setRefXY(ref_list, 2, 1, info->refs[0][2][0]);
		setRefXY(ref_list, 3, 1, info->refs[0][2][0]);
		break;
	case MB_8x8:
	case MB_8x8ref0:
		for (i = 0; i < 4; ++i)
		{
			switch (info->submb_part[i])
			{
			case MB_8x8: eg_write_ue(bs, 0); break;
			case MB_8x4: eg_write_ue(bs, 1); break;
			case MB_4x8: eg_write_ue(bs, 2); break;
			case MB_4x4: eg_write_ue(bs, 3); break;
			default :
				assert(0);
			}
		}
		info->mb_part = MB_8x8;
		for (i = 0; i < 4; ++i)
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
				getRefXY(ref_list, xx, yy, yy+2, predV, i<3);
				eg_write_se(bs, info->refs[xx][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy][0].x - predV.x);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx][yy][0]);
				break;
			case MB_8x4:
				getRefXY(ref_list, xx, yy, yy+1, predV, i<3);
				eg_write_se(bs, info->refs[xx][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy][0].x - predV.x);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx, yy+1, info->refs[xx][yy][0]);

				if (i&1)
					predV = info->refs[xx][yy-1][0];
				else
					copyVector(predV, ref_list->lefts[xx]);
				getRefXY(ref_list, xx+1, yy, yy+2, predV, false);
				eg_write_se(bs, info->refs[xx][yy+1][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy+1][0].x - predV.x);
				setRefXY(ref_list, xx+1, yy, info->refs[xx][yy+1][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx][yy+1][0]);
				break;
			case MB_4x8:
				getRefXY(ref_list, xx, yy, yy+2, predV, true);
				eg_write_se(bs, info->refs[xx][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy][0].x - predV.x);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);
				setRefXY(ref_list, xx+1, yy, info->refs[xx][yy][0]);

				if (i<2)
					copyVector(predV, ref_list->tops[yy]);
				else
					predV = info->refs[xx-1][yy][0];
				getRefXY(ref_list, xx, yy+1, yy+2, predV, i<3);
				eg_write_se(bs, info->refs[xx+1][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx+1][yy][0].x - predV.x);
				setRefXY(ref_list, xx, yy+1, info->refs[xx+1][yy][0]);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx+1][yy][0]);

				break;
			case MB_4x4:
				getRefXY(ref_list, xx, yy, yy+1, predV, true);
				eg_write_se(bs, info->refs[xx][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy][0].x - predV.x);
				setRefXY(ref_list, xx, yy, info->refs[xx][yy][0]);

				if (i<2)
					copyVector(predV, ref_list->tops[yy]);
				else
					predV = info->refs[xx-1][yy][0];
				getRefXY(ref_list, xx, yy+1, yy+2, predV, i<3);
				eg_write_se(bs, info->refs[xx][yy+1][0].y - predV.y);
				eg_write_se(bs, info->refs[xx][yy+1][0].x - predV.x);
				setRefXY(ref_list, xx, yy+1, info->refs[xx][yy+1][0]);

				if (i&1)
					predV = info->refs[xx][yy-1][0];
				else
					copyVector(predV, ref_list->lefts[xx]);
				getRefXY(ref_list, xx+1, yy, yy+1, predV, true);
				eg_write_se(bs, info->refs[xx+1][yy][0].y - predV.y);
				eg_write_se(bs, info->refs[xx+1][yy][0].x - predV.x);
				setRefXY(ref_list, xx+1, yy, info->refs[xx][yy][0]);

				predV = info->refs[xx][yy][0];
				getRefXY(ref_list, xx+1, yy+1, yy+2, predV, false);
				eg_write_se(bs, info->refs[xx+1][yy+1][0].y - predV.y);
				eg_write_se(bs, info->refs[xx+1][yy+1][0].x - predV.x);
				setRefXY(ref_list, xx+1, yy+1, info->refs[xx][yy+1][0]);

				break;
			}
		}
		// todo:
		break;
	default:
		assert(0);
	}
	/*
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			int x = i / 2 * 2 + j / 2;
			int y = i % 2 * 2 + j % 2;
			if (info->refs[x][y][0].refno >= 0)
			{
				eg_write_se(bs, info->refs[x][y][0].x - 0);//info->ref_ref.x);
				eg_write_se(bs, info->refs[x][y][0].y - 0);//info->ref_ref.y);
				//printf("MVd %d, %d\n", info->refs[x][y][1].x - info->ref_ref.x,
				//	info->refs[x][y][1].y - info->ref_ref.y);
			}
		}
	}
	*/
	macroblock_write_cavlc_PB_y(bs, info, ref_list);
	macroblock_write_cavlc_uv(bs, info, ref_list);
}

void macroblock_write_cavlc_B_slice(Bitstream *bs, pmacroblockInfo info, CodingPictInfo *cpi, cavlc_ref_list *ref_list)
{
	int skip = 5;
	int i;

	if (info->mb_part==MB_8x8 || info->mb_part==MB_8x8ref0)
	{
		info->mb_part = MB_8x8ref0;
		/*		// PSlice, only reference 1 frame
		if (info->ref[0].refno==0 &&
		info->ref[4].refno==0 &&
		info->ref[8].refno==0 &&
		info->ref[12].refno==0)
		info->mb_part = MB_8x8ref0;
		else
		info->mb_part = MB_8x8;
		*/
	}
	//eg_write_ue(bs, info->mb_part);  /* skip run */
	skip = 0;
	//eg_write_ue(bs, skip);  /* skip run */
	//skip = 0;
	//refVector vec;
	uint32_t RN = 0;
	switch (info->mb_part)
	{
	case MB_16x16:
		if (info->mb_part2==B_L0_16x16 && info->refs[0][0][0].x==0 && info->refs[0][0][0].y==0 && info->refs[0][0][0].refno==0)
		{
			eg_write_ue(bs, 0);
			RN = 0;
		}
		else
		{
			switch (info->mb_part2)
			{
			case B_L0_16x16:
				eg_write_ue(bs, 1);
				RN = 0x00000001;
				break;
			case B_L1_16x16:
				eg_write_ue(bs, 2);
				RN = 0x00010000;
				break;
			case B_Bi_16x16:
				eg_write_ue(bs, 3);
				RN = 0x00010001;
				break;
			default :
				assert(0);
			}
		}
		break;
	case MB_16x8:
		eg_write_ue(bs, info->mb_part2);//B_L0_L0_16x8);//info->mb_part2);//B_L0_L0_16x8,B_L0_L1_16x8,B_L0_Bi_16x8,...,B_Bi_Bi_16x8
		switch (info->mb_part2)
		{
		case B_L0_L0_16x8:
			RN = 0x00000011;
			break;
		case B_L0_L1_16x8:
			RN = 0x00100001;
			break;
		case B_L0_Bi_16x8:
			RN = 0x00100011;
			break;
		case B_L1_L0_16x8:
			RN = 0x00010010;
			break;
		case B_L1_L1_16x8:
			RN = 0x00110000;
			break;
		case B_L1_Bi_16x8:
			RN = 0x00110010;
			break;
		case B_Bi_L0_16x8:
			RN = 0x00010011;
			break;
		case B_Bi_L1_16x8:
			RN = 0x00110001;
			break;
		case B_Bi_Bi_16x8:
			RN = 0x00110011;
			break;
		default:
			assert(0);
		}
		break;
	case MB_8x16:
		eg_write_ue(bs, info->mb_part2);//B_L0_L0_16x8);//info->mb_part2);//B_L0_L0_16x8,B_L0_L1_16x8,B_L0_Bi_16x8,...,B_Bi_Bi_16x8
		switch (info->mb_part2)
		{
		case B_L0_L0_8x16:
			RN = 0x00000101;
			break;
		case B_L0_L1_8x16:
			RN = 0x01000001;
			break;
		case B_L0_Bi_8x16:
			RN = 0x01000101;
			break;
		case B_L1_L0_8x16:
			RN = 0x00010100;
			break;
		case B_L1_L1_8x16:
			RN = 0x01010000;
			break;
		case B_L1_Bi_8x16:
			RN = 0x01010100;
			break;
		case B_Bi_L0_8x16:
			RN = 0x00010101;
			break;
		case B_Bi_L1_8x16:
			RN = 0x01010001;
			break;
		case B_Bi_Bi_8x16:
			RN = 0x01010101;
			break;
		default:
			assert(0);
		}
		break;
	case MB_8x8:
	case MB_8x8ref0:
		eg_write_ue(bs, 22);
		RN = 0x00000000;
		for (i = 0; i < 4; ++i)
		{
			int v = 0;
			switch (info->submb_part[i])
			{
			case MB_8x8: v = 1; RN |= 0x01 << (i<<2); break;
			case MB_8x4: v = 4; RN |= 0x03 << (i<<2); break;
			case MB_4x8: v = 5; RN |= 0x05 << (i<<2); break;
			case MB_4x4: v = 10; RN |= 0x0F << (i<<2); break;
			default :
				assert(0);
			}
			eg_write_ue(bs, v);
		}
		break;
	default:
		assert(0);
	}
	int rn = RN;
	if (cpi->Ref_Count[0]>1)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (rn & 0x0F)
			{
				int x = i / 2 * 2;
				int y = i % 2 * 2;
				eg_write_te(bs, cpi->Ref_Count[0]-1, info->refs[x][y][0].refno);
			}
			rn >>= 4;
		}
	}
	else
		rn >>= 16;
	if (cpi->Ref_Count[1]>1)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (rn & 0x0F)
			{
				int x = i / 2 * 2;
				int y = i % 2 * 2;
				eg_write_te(bs, cpi->Ref_Count[1]-1, info->refs[x][y][1].refno);
			}
			rn >>= 4;
		}
	}
	rn = RN;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (rn & 1)
			{
				int x = i / 2 * 2 + j / 2;
				int y = i % 2 * 2 + j % 2;
				eg_write_se(bs, info->refs[x][y][0].y - 0);//info->ref_ref.y);
				eg_write_se(bs, info->refs[x][y][0].x - 0);//info->ref_ref.x);
			}
			rn >>= 1;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (rn & 1)
			{
				int x = i / 2 * 2 + j / 2;
				int y = i % 2 * 2 + j % 2;
				eg_write_se(bs, info->refs[x][y][1].y - 0);//info->ref_ref.y);
				eg_write_se(bs, info->refs[x][y][1].x - 0);//info->ref_ref.x);
			}
			rn >>= 1;
		}
	}
	macroblock_write_cavlc_PB_y(bs, info, ref_list);
	macroblock_write_cavlc_uv(bs, info, ref_list);
}

