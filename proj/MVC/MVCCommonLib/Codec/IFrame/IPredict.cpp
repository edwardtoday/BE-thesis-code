/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Create
 * 
 ************************************************************************/

#include "IPredict.h"
#include "stdlib.h"
#include "Codec/Consts4Standard.h"

void do_4x4_luma_prediction(info_4x4_luma_prediction ind, out_4x4_luma_prediction oud, uint8_t tp)
{
	register const uint8 *ip = ind;
	register uint8 *op = oud[0];
	switch (tp)
	{
	case 0: {//Mode 0: Vertical
			*(op++) = ip[0]; *(op++) = ip[1]; *(op++) = ip[2]; *(op++) = ip[3];
			*(op++) = ip[0]; *(op++) = ip[1]; *(op++) = ip[2]; *(op++) = ip[3];
			*(op++) = ip[0]; *(op++) = ip[1]; *(op++) = ip[2]; *(op++) = ip[3];
			*(op++) = ip[0]; *(op++) = ip[1]; *(op++) = ip[2]; *(op++) = ip[3];
			break;
		}
	case 1: {//Mode 1: Horizontal
			ip += 8;
			op[0] = op[1] = op[2] = op[3] = *(ip++);
			op[4] = op[5] = op[6] = op[7] = *(ip++);
			op[8] = op[9] = op[10] = op[11] = *(ip++);
			op[12] = op[13] = op[14] = op[15] = *(ip++);
			break;
		}
	case 2: {//Mode 2: DC
			int tmp = 0;
			tmp += *(ip++); tmp += *(ip++); tmp += *(ip++); tmp += *(ip++);
			ip += 4;
			tmp += *(ip++); tmp += *(ip++); tmp += *(ip++); tmp += *(ip++);
			tmp = (tmp + 4) >> 3;
			memset(op, (uint8)tmp, 16);
			break;
		}
	case 21: {//Mode 20 DC top
			int tmp = 0;
			tmp += *(ip++); tmp += *(ip++); tmp += *(ip++); tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op, (uint8)tmp, 16);
			break;
		}
	case 22: {//Mode 20 DC left
			int tmp = 0;
			ip += 8;
			tmp += *(ip++); tmp += *(ip++); tmp += *(ip++); tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op, (uint8)tmp, 16);
			break;
		}
	case 23: {//Mode 23 DC 128
			memset(op, 128, 16);
			break;
		}
	case 3: {//Mode 3 Intra_4x4_DIAGONAL_DOWNLEFT when Top are available
			op[0] = ((int)ip[0] + ((int)ip[1]<<1) + (int)ip[2] + 2) >> 2;
			op[4] = op[1] = ((int)ip[1] + ((int)ip[2]<<1) + (int)ip[3] + 2) >> 2;
			op[8] = op[5] = op[2] = ((int)ip[2] + ((int)ip[3]<<1) + (int)ip[4] + 2) >> 2;
			op[12] = op[9] = op[6] = op[3] = ((int)ip[3] + ((int)ip[4]<<1) + (int)ip[5] + 2) >> 2;
			op[13] = op[10] = op[7] = ((int)ip[4] + ((int)ip[5]<<1) + (int)ip[6] + 2) >> 2;
			op[14] = op[11] = ((int)ip[5] + ((int)ip[6]<<1) + (int)ip[7] + 2) >> 2;
			op[15] = ((int)ip[6] + ((int)ip[7]<<1) + (int)ip[7] + 2) >> 2;
			break;
		}
	case 4: {//Mode 4 Intra_4x4_DIAGONAL_DOWNRIGHT when Top and left are available
			op[15] = op[10] = op[5] = op[0] = ((int)ip[8] + ((int)ip[12]<<1) + (int)ip[0] + 2) >> 2;
			op[11] = op[6] = op[1] = ((int)ip[12] + ((int)ip[0]<<1) + (int)ip[1] + 2) >> 2;
			op[7] = op[2] = ((int)ip[0] + ((int)ip[1]<<1) + (int)ip[2] + 2) >> 2;
			op[3] = ((int)ip[1] + ((int)ip[2]<<1) + (int)ip[3] + 2) >> 2;
			op[14] = op[9] = op[4] = ((int)ip[9] + ((int)ip[8]<<1) + (int)ip[12] + 2) >> 2;
			op[13] = op[8] = ((int)ip[10] + ((int)ip[9]<<1) + (int)ip[8] + 2) >> 2;
			op[12] = ((int)ip[11] + ((int)ip[10]<<1) + (int)ip[9] + 2) >> 2;
			break;
		}
	case 5: {//Mode 5 Intra_4x4_VERTICAL_RIGHT when Top and left are available
			op[9] = op[0] = ((int)ip[12] + (int)ip[0] + 1) >> 1;
			op[10] = op[1] = ((int)ip[0] + (int)ip[1] + 1) >> 1;
			op[11] = op[2] = ((int)ip[1] + (int)ip[2] + 1) >> 1;
			op[3] = ((int)ip[2] + (int)ip[3] + 1) >> 1;
			op[13] = op[4] = ((int)ip[8] + ((int)ip[12]<<1) + (int)ip[0] + 2) >> 2;
			op[14] = op[5] = ((int)ip[12] + ((int)ip[0]<<1) + (int)ip[1] + 2) >> 2;
			op[15] = op[6] = ((int)ip[0] + ((int)ip[1]<<1) + (int)ip[2] + 2) >> 2;
			op[7] = ((int)ip[1] + ((int)ip[2]<<1) + (int)ip[3] + 2) >> 2;
			op[8] = ((int)ip[9] + ((int)ip[8]<<1) + (int)ip[12] + 2) >> 2;
			op[12] = ((int)ip[10] + ((int)ip[9]<<1) + (int)ip[8] + 2) >> 2;
			break;
		}
	case 6: {//Mode 6 Intra_4x4_HORIZONTAL_DOWN when Top and left are available
			op[6] = op[0] = ((int)ip[8] + (int)ip[12] + 1) >> 1;
			op[7] = op[1] = ((int)ip[8] + ((int)ip[12]<<1) + (int)ip[0] + 2) >> 2;
			op[2] = ((int)ip[12] + ((int)ip[0]<<1) + (int)ip[1] + 2) >> 2;
			op[3] = ((int)ip[0] + ((int)ip[1]<<1) + (int)ip[2] + 2) >> 2;
			op[10] = op[4] = ((int)ip[9] + (int)ip[8] + 1) >> 1;
			op[11] = op[5] = ((int)ip[9] + ((int)ip[8]<<1) + (int)ip[12] + 2) >> 2;
			op[14] = op[8] = ((int)ip[10] + (int)ip[9] + 1) >> 1;
			op[15] = op[9] = ((int)ip[10] + ((int)ip[9]<<1) + (int)ip[8] + 2) >> 2;
			op[12] = ((int)ip[11] + (int)ip[10] + 1) >> 1;
			op[13] = ((int)ip[11] + ((int)ip[10]<<1) + (int)ip[9] + 2) >> 2;
			break;
		}
	case 7: {//Mode 7 Intra_4x4_VERTICAL_LEFT when Top are available
			op[0] = ((int)ip[0] + (int)ip[1] + 1) >> 1;
			op[8] = op[1] = ((int)ip[1] + (int)ip[2] + 1) >> 1;
			op[9] = op[2] = ((int)ip[2] + (int)ip[3] + 1) >> 1;
			op[10] = op[3] = ((int)ip[3] + (int)ip[4] + 1) >> 1;
			op[11] = ((int)ip[4] + (int)ip[5] + 1) >> 1;
			op[4] = ((int)ip[0] + ((int)ip[1]<<1) + (int)ip[2] + 2) >> 2;
			op[12] = op[5] = ((int)ip[1] + ((int)ip[2]<<1) + (int)ip[3] + 2) >> 2;
			op[13] = op[6] = ((int)ip[2] + ((int)ip[3]<<1) + (int)ip[4] + 2) >> 2;
			op[14] = op[7] = ((int)ip[3] + ((int)ip[4]<<1) + (int)ip[5] + 2) >> 2;
			op[15] = ((int)ip[4] + ((int)ip[5]<<1) + (int)ip[6] + 2) >> 2;
			break;
		}
	case 8: {//Mode 8 Intra_4x4_HORIZONTAL_UP when Left are available
			op[0] = ((int)ip[8] + (int)ip[9] + 1) >> 1;
			op[1] = ((int)ip[8] + ((int)ip[9]<<1) + (int)ip[10] + 2) >> 2;
			op[4] = op[2] = ((int)ip[9] + (int)ip[10] + 1) >> 1;
			op[5] = op[3] = ((int)ip[9] + ((int)ip[10]<<1) + (int)ip[11] + 2) >> 2;
			op[8] = op[6] = ((int)ip[10] + (int)ip[11] + 1) >> 1;
			op[9] = op[7] = ((int)ip[10] + ((int)ip[11]<<1) + (int)ip[11] + 2) >> 2;
			op[10] = op[11] = op[12] = op[13] = op[14] = op[15] = ip[11];
			break;
		}
	}
}

void do_8x8_luma_prediction(uint8_t *top, uint8_t *left, int topleft, out_8x8_luma_prediction oud, uint8_t tp)
{
	// Why
//	if (tp!=2 && (tp/10)!=2)
	{
		int newtl = (top[0] + topleft * 2 + left[0] + 2) >> 2;
		int last = topleft;
		for (int i = 0; i < 15; ++i)
		{
			int cur = (last + top[i] * 2 + top[i+1] + 2) >> 2;
			last = top[i];
			top[i] = cur;
		}
		top[15] = (last + top[15] * 3 + 2) >> 2;
		top[16] = top[15];
		last = topleft;
		for (int i = 0; i < 7; ++i)
		{
			int cur = (last + left[i] * 2 + left[i+1] + 2) >> 2;
			last = left[i];
			left[i] = cur;
		}
		left[7] = (last + left[7] * 3 + 2) >> 2;
		topleft = newtl;
	}

	register uint8 *op = oud[0];
	switch (tp)
	{
	case 0: {//Mode 0: Vertical
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				oud[i][j] = top[j];
		break;
			}
	case 1: {//Mode 1: Horizontal
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				oud[i][j] = left[i];
		break;
			}
	case 2: {//Mode 2: DC
		int tmp = 0;
		for (int i = 0; i < 8; ++i)
			tmp += left[i];
		for (int i = 0; i < 8; ++i)
			tmp += top[i];
		tmp = (tmp + 8) >> 4;
		memset(op, (uint8)tmp, 64);
		break;
			}
	case 21: {//Mode 20 DC top
		int tmp = 0;
		for (int i = 0; i < 8; ++i)
			tmp += top[i];
		tmp = (tmp + 4) >> 3;
		memset(op, (uint8)tmp, 64);
		break;
			 }
	case 22: {//Mode 20 DC left
		int tmp = 0;
		for (int i = 0; i < 8; ++i)
			tmp += left[i];
		tmp = (tmp + 4) >> 3;
		memset(op, (uint8)tmp, 64);
		break;
			 }
	case 23: {//Mode 23 DC 128
		memset(op, 128, 64);
		break;
			 }
	case 3: {//Mode 3 Intra_4x4_DIAGONAL_DOWNLEFT when Top are available
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				oud[i][j] = ((int)top[j+i] + ((int)top[j+i+1]<<1) + (int)top[j+i+2] + 2) >> 2;
		break;
			}
	case 4: {//Mode 4 Intra_4x4_DIAGONAL_DOWNRIGHT when Top and left are available
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < i - 1; ++j)
				oud[i][j] = ((int)left[i-j-2] + ((int)left[i-j-1]<<1) + (int)left[i-j] + 2) >> 2;
		for (int i = 0; i < 8; ++i)
			for (int j = i + 2; j < 8; ++j)
				oud[i][j] = ((int)top[j-i-2] + ((int)top[j-i-1]<<1) + (int)top[j-i] + 2) >> 2;
		for (int i = 0; i < 8; ++i)
			oud[i][i] = ((int)left[0] + ((int)topleft<<1) + (int)top[0] + 2) >> 2;
		for (int i = 0; i < 7; ++i)
			oud[i][i+1] = ((int)top[1] + ((int)top[0]<<1) + (int)topleft + 2) >> 2;
		for (int i = 0; i < 7; ++i)
			oud[i+1][i] = ((int)left[1] + ((int)left[0]<<1) + (int)topleft + 2) >> 2;
		break;
			}
	case 5:
		{//Mode 5 Intra_4x4_VERTICAL_RIGHT when Top and left are available
			int16 s[26];
			for (int i = 0; i < 7; ++i)
				s[8+2+i*2] = (top[i] + top[i+1] + 1) >> 1;
			for (int i = 0; i < 6; ++i)
				s[8+2+i*2+1] = (top[i] + 2 * top[i+1] + top[i+2] + 2) >> 2;
			s[23] = (top[6] + 2 * top[7] + top[7] + 2) >> 2;;
			s[24] = top[7];
			s[25] = top[7];
			s[8] = (topleft + top[0] + 1) >> 1;
			s[9] = (topleft + top[0] * 2 + top[1] + 2) >> 2;
			s[7] = (top[0] + topleft * 2 + left[0] + 2) >> 2;
			s[6] = (topleft + left[0] * 2 + left[1] + 2) >> 2;
			for (int i = 0; i < 6; ++i)
				s[5-i] = (left[i] + 2 * left[i+1] + left[i+2] + 2) >> 2;
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j)
					oud[i][j] = (uint8)s[j * 2 - i + 8];
			break;
		}
	case 6:
		{//Mode 6 Intra_4x4_HORIZONTAL_DOWN when Top and left are available
			int s[24];
			for (int i = 0; i < 7; ++i)
				s[8+2+i*2] = (left[i] + left[i+1] + 1) >> 1;
			for (int i = 0; i < 6; ++i)
				s[8+2+i*2+1] = (left[i] + 2 * left[i+1] + left[i+2] + 2) >> 2;
			s[8] = (topleft + left[0] + 1) >> 1;
			s[9] = (topleft + left[0] * 2 + left[1] + 2) >> 2;
			s[7] = (left[0] + topleft * 2 + top[0] + 2) >> 2;
			s[6] = (topleft + top[0] * 2 + top[1] + 2) >> 2;
			for (int i = 0; i < 6; ++i)
				s[5-i] = (top[i] + 2 * top[i+1] + top[i+2] + 2) >> 2;
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j)
					oud[i][j] = s[i * 2 - j + 8];
			break;
		}
	case 7:
		{//Mode 7 Intra_4x4_VERTICAL_LEFT when Top are available
			int s[24];
			for (int i = 0; i < 11; ++i)
				s[i*2] = (top[i] + top[i+1] + 1) >> 1;
			for (int i = 0; i < 11; ++i)
				s[i*2+1] = (top[i] + 2 * top[i+1] + top[i+2] + 2) >> 2;
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j)
					oud[i][j] = s[j * 2 + i];
			break;
		}
	case 8:
		{//Mode 8 Intra_4x4_HORIZONTAL_UP when Left are available
			int16 s[24];
			for (int i = 0; i < 7; ++i)
				s[i*2] = (left[i] + left[i+1] + 1) >> 1;
			for (int i = 0; i < 6; ++i)
				s[i*2+1] = (left[i] + 2 * left[i+1] + left[i+2] + 2) >> 2;
			s[13] = (left[6] + 3 * left[7] + 2) >> 2;
			for (int i = 14; i < 24; ++i)
				s[i] = left[7];
			for (int i = 0; i < 8; ++i)
				for (int j = 0; j < 8; ++j)
					oud[i][j] = (uint8)s[i * 2 + j];
			break;
		}
	}
}

void enum_16x16_luma_prediction(int32_t hasLeft, int32_t hasTop, uint8_t *modes, int32_t *modeCount)
{
	if (hasLeft && hasTop)
	{
		*modeCount = 4;
		modes[0] = Intra_16x16_TOP;
		modes[1] = Intra_16x16_LEFT;
		modes[2] = Intra_16x16_DC;
		modes[3] = Intra_16x16_PLANE;
	}
	else if (hasLeft)
	{
		*modeCount = 2;
		modes[0] = Intra_16x16_LEFT;
		modes[1] = Intra_16x16_DCLEFT;
	}
	else if (hasTop)
	{
		*modeCount = 2;
		modes[0] = Intra_16x16_TOP;
		modes[1] = Intra_16x16_DCTOP;
	}
	else
	{
		*modeCount = 1;
		modes[0] = Intra_16x16_DC128;
	}
}

uint8_t best_16x16_luma_prediction(predictInfo *pif, uint8_t *modes, int32_t modeCount, macroblockdata *dlist)
{
	int bestModeNum = 0;
	int minDiff = 0;
	int diff = 0;
	out_16x16_luma_prediction tempoud;

	do_16x16_luma_prediction(pif, tempoud, modes[0]);

	for (int i=0; i<16; i++)
		for (int j=0; j<16; j++)
			diff += abs(tempoud[i][j] - dlist->y[i][j]);
	minDiff = diff;
	for(int k=1; k<modeCount; k++) {
		diff = 0;
		do_16x16_luma_prediction(pif, tempoud, modes[k]);
	
		for (int i=0; i<16; i++)
			for (int j=0; j<16; j++)
				diff += abs(tempoud[i][j] - dlist->y[i][j]);
		if (diff < minDiff) {
			bestModeNum = k;
			minDiff = diff;
		}
	}

	return modes[bestModeNum];
}

void do_16x16_luma_prediction(predictInfo *pif, out_16x16_luma_prediction oud, uint8_t tp)
{
	register uint8 *op = oud[0];
	int i, j;
	switch (tp)
	{
	case Intra_16x16_TOP: {//Mode 0: Vertical
			const uint8 *ip = pif->top;
			memcpy(op, ip, 16);
			memcpy(op+16, op, 16);
			memcpy(op+32, op, 32);
			memcpy(op+64, op, 64);
			memcpy(op+128, op, 128);
			break;
		}
	case Intra_16x16_LEFT: {//Mode 1: Horizontal
			const uint8 *ip = pif->left;
			for (i = 0; i < 16; ++i)
			{
				register uint8 tmp = *(ip++);
				for (j = 0; j < 16; ++j)
					*(op++) = tmp;
			}
			break;
		}
	case Intra_16x16_DC: {//Mode 2: DC
			int tmp = 0;
			const uint8 *ip = pif->top;
			for (i = 0; i < 16; ++i)
				tmp += *(ip++);
			ip = pif->left;
			for (i = 0; i < 16; ++i)
				tmp += *(ip++);
			tmp = (tmp + 16) >> 5;
			memset(op, (uint8)tmp, 16*16);
			break;
		}
	case Intra_16x16_DCTOP: {//Mode 20 DC top
			int tmp = 0;
			const uint8 *ip = pif->top;
			for (i = 0; i < 16; ++i)
				tmp += *(ip++);
			tmp = (tmp + 8) >> 4;
			memset(op, (uint8)tmp, 16*16);
			break;
		}
	case Intra_16x16_DCLEFT: {//Mode 20 DC left
			int tmp = 0;
			const uint8 *ip = pif->left;
			for (i = 0; i < 16; ++i)
				tmp += *(ip++);
			tmp = (tmp + 8) >> 4;
			memset(op, (uint8)tmp, 16*16);
			break;
		}
	case Intra_16x16_DC128: {//Mode 23 DC 128
			memset(op, 128, 16*16);
			break;
		}
	case Intra_16x16_PLANE: {//Mode 3: Plane
			int H = 0, V = 0;
			int aa, bb, cc;
			const uint8 *ip = pif->top;
			const uint8 *ip2 = pif->left;
			
			for(i = 1; i <= 7; ++i)
			{
				H += i * ((int)ip[7 + i] - (int)ip[7 - i]);
				V += i * ((int)ip2[7 + i] - (int)ip2[7 - i]);
			}
			H += 8 * ((int)ip[15] - (int)pif->topleft);
			V += 8 * ((int)ip2[15] - (int)pif->topleft);
			
			aa = (ip[15] + ip2[15]) << 4;
			bb = (5 * H + 32) >> 6;
			cc = (5 * V + 32) >> 6;
			
			for(i = 0 ; i < 16 ; ++i)
				for(j = 0 ; j < 16 ; ++j)
				{
					int tmp = (aa + bb * (j - 7) + cc * (i - 7) + 16) >> 5;
					*(op++) = CLIP1(tmp);
				}
			break;
		}
	}
}

uint8_t macroblock_luma_prediction(int32_t hasLeft, int32_t hasTop, predictInfo *pif, out_16x16_luma_prediction oud, macroblockdata *dlist)
{
	/*static */
	uint8_t modes[8];
	int32_t modeCount;
	uint8_t bestMode;
	enum_16x16_luma_prediction(hasLeft, hasTop, modes, &modeCount);
	bestMode = best_16x16_luma_prediction(pif, modes, modeCount, dlist);
	do_16x16_luma_prediction(pif, oud, bestMode);
	return bestMode;
}

void enum_8x8_chroma_prediction(int32_t hasLeft, int32_t hasTop, uint8_t *modes, int32_t *modeCount)
{
	if (hasLeft && hasTop)
	{
		*modeCount = 4;
		modes[0] = Intra_8x8_TOP;
		modes[1] = Intra_8x8_LEFT;
		modes[2] = Intra_8x8_DC;
		modes[3] = Intra_8x8_PLANE;
	}
	else
	if (hasLeft)
	{
		*modeCount = 2;
		modes[0] = Intra_8x8_LEFT;
		modes[1] = Intra_8x8_DCLEFT;
	}
	else
	if (hasTop)
	{
		*modeCount = 2;
		modes[0] = Intra_8x8_TOP;
		modes[1] = Intra_8x8_DCTOP;
	}
	else
	{
		*modeCount = 1;
		modes[0] = Intra_8x8_DC128;
	}
}

uint8_t best_8x8_chroma_prediction(const info_8x8_chroma_prediction ind_u, const info_8x8_chroma_prediction ind_v, uint8_t *modes, int32_t modeCount, macroblockdata *dlist)
{
	int bestModeNum = 0;
	int minDiff = 0;
	int diff = 0;
	out_8x8_chroma_prediction tempoud_u, tempoud_v;

	do_8x8_chroma_prediction(ind_u, tempoud_u, modes[0]);
	do_8x8_chroma_prediction(ind_v, tempoud_v, modes[0]);

	for (int i=0; i<16; i++)
		for (int j=0; j<16; j++) {
			diff += abs(tempoud_u[i][j] - dlist->uv[0][i][j]);
			diff += abs(tempoud_v[i][j] - dlist->uv[1][i][j]);
		}

	minDiff = diff;
	for(int k=1; k<modeCount; k++) {
		diff = 0;
		do_8x8_chroma_prediction(ind_u, tempoud_u, modes[k]);
		do_8x8_chroma_prediction(ind_v, tempoud_v, modes[k]);
	
		for (int i=0; i<16; i++)
			for (int j=0; j<16; j++) {
				diff += abs(tempoud_u[i][j] - dlist->uv[0][i][j]);
				diff += abs(tempoud_v[i][j] - dlist->uv[1][i][j]);
			}
		if (diff < minDiff) {
			bestModeNum = k;
			minDiff = diff;
		}
	}

	return modes[bestModeNum];
}

void do_8x8_chroma_prediction(const info_8x8_chroma_prediction ind, out_8x8_chroma_prediction oud, uint8_t tp)
{
	register const uint8 *ip = ind;
	register uint8 *op = oud[0];
	int i, j;
	switch (tp)
	{
	case Intra_8x8_TOP: {//Mode 2: Vertical
			memcpy(op, ip, 8);
			memcpy(op+8, op, 8);
			memcpy(op+16, op, 16);
			memcpy(op+32, op, 32);
			break;
		}
	case Intra_8x8_LEFT: {//Mode 1: Horizontal
			ip += 8;
			for (i = 0; i < 8; ++i)
			{
				register uint8 tmp = *(ip++);
				for (j = 0; j < 8; ++j)
					*(op++) = tmp;
			}
			break;
		}
	case Intra_8x8_DC: {//Mode 0: DC
			int s0 = 0, s1 = 0, s2 = 0, s3 = 0;
			for (i = 0; i < 4; ++i)
				s0 += *(ip++);
			for (i = 0; i < 4; ++i)
				s1 += *(ip++);
			for (i = 0; i < 4; ++i)
				s2 += *(ip++);
			for (i = 0; i < 4; ++i)
				s3 += *(ip++);
			op[0] = op[1] = op[2] = op[3] = (s0 + s2 + 4) >> 3;
			op[4] = op[5] = op[6] = op[7] = (s1 + 2) >> 2;
			for (i = 0; i < 3; ++i)
				memcpy(op+i*8+8, op, 8);
			op += 32;
			op[0] = op[1] = op[2] = op[3] = (s3 + 2) >> 2;
			op[4] = op[5] = op[6] = op[7] = (s3 + s1 + 4) >> 3;
			for (i = 0; i < 3; ++i)
				memcpy(op+i*8+8, op, 8);
			break;
		}
	case Intra_8x8_DCTOP: {//Mode 01 DC top
			int tmp = 0;
			for (i = 0; i < 4; ++i)
				tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op, (uint8)tmp, 4);
			tmp = 0;
			for (i = 0; i < 4; ++i)
				tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op+4, (uint8)tmp, 4);
			for (i = 0; i < 7; ++i)
				memcpy(op+i*8+8,op,8);
			break;
		}
	case Intra_8x8_DCLEFT: {//Mode 02 DC left
			int tmp = 0;
			ip += 8;
			for (i = 0; i < 4; ++i)
				tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op, (uint8)tmp, 4*8);
			
			tmp = 0;
			for (i = 0; i < 4; ++i)
				tmp += *(ip++);
			tmp = (tmp + 2) >> 2;
			memset(op+32, (uint8)tmp, 4*8);
			break;
		}
	case Intra_8x8_DC128: {//Mode 03 DC 128
			memset(op, 128, 8*8);
			break;
		}
	case Intra_8x8_PLANE: {//Mode 3: Plane
			int H = 0, V = 0;
			int a, b, c;
			
			for(i = 1; i <= 3; ++i)
			{
				H += i * ((int)ip[3 + i] - (int)ip[3 - i]);
				V += i * ((int)ip[11 + i] - (int)ip[11 - i]);
			}
			H += 4 * ((int)ip[7] - (int)ip[16]);
			V += 4 * ((int)ip[15] - (int)ip[16]);
			
			a = (ip[7] + ip[15]) << 4;
			b = (17 * H + 16) >> 5;
			c = (17 * V + 16) >> 5;
			
			for(i = 0 ; i < 8 ; ++i)
				for(j = 0 ; j < 8 ; ++j)
				{
					int tmp = (a + b * (j - 3) + c * (i - 3) + 16) >> 5;
					*(op++) = CLIP1(tmp);
				}
			break;
		}
	}
}

uint8_t macroblock_chroma_prediction(int32_t hasLeft, int32_t hasTop, const info_8x8_chroma_prediction ind_u, const info_8x8_chroma_prediction ind_v, out_8x8_chroma_prediction oud_u, out_8x8_chroma_prediction oud_v, macroblockdata *dlist)
{
	/*static */
	uint8_t modes[8];
	int32_t modeCount;
	uint8_t bestMode;
	enum_8x8_chroma_prediction(hasLeft, hasTop, modes, &modeCount);
	bestMode = best_8x8_chroma_prediction(ind_u, ind_v, modes, modeCount, dlist);
	do_8x8_chroma_prediction(ind_u, oud_u, bestMode);
	do_8x8_chroma_prediction(ind_v, oud_v, bestMode);
	return bestMode;
}

void macroblock_prediction(predictInfo *pinfo, macroblockdata *pred, uint8_t mode_y, uint8_t mode_uv)
{
	do_16x16_luma_prediction(pinfo, pred->y, mode_y);
	do_8x8_chroma_prediction(pinfo->data_u, pred->uv[0], mode_uv);
	do_8x8_chroma_prediction(pinfo->data_v, pred->uv[1], mode_uv);
}

void macroblock_prediction_getbest(predictInfo *pinfo, macroblockdata *pred, uint8_t *mode_y, uint8_t *mode_uv, macroblockdata *dlist)
{
	*mode_y = macroblock_luma_prediction(pinfo->hasLeft, pinfo->hasTop, pinfo, pred->y, dlist);
	*mode_uv = macroblock_chroma_prediction(pinfo->hasLeft, pinfo->hasTop, pinfo->data_u, pinfo->data_v, pred->uv[0], pred->uv[1], dlist);
}

