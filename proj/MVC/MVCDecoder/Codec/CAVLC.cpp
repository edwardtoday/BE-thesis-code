/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Modified from T264
 * 
 ************************************************************************/

#include "CAVLC.h"
#include "Codec/Bitstream.h"
#include "Codec/vlc.h"
#include "Common.h"
#include "Codec/Consts4Standard.h"

#ifndef assert
#define assert ASSERT
#endif // assert

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

/*static const int8_t luma_index[] = 
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

#ifdef NO_COMP


#endif // NOCOMP

/* nC == -1 */
void T264dec_mb_read_coff_token_t4(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff)
{
	int32_t code;

	code = eg_show(bs, 8);
	if (code >= 16)
	{
		if (code >= 128)
		{
			/* 1 */
			*trailing_ones = 1;
			*total_coff = 1;
			eg_read_skip(bs, 1);
		}
		else if (code >= 64)
		{
			/* 01 */
			*trailing_ones = 0;
			*total_coff = 0;
			eg_read_skip(bs, 2);
		}
		else if (code >= 32)
		{
			/* 001 */
			*trailing_ones = 2;
			*total_coff = 2;
			eg_read_skip(bs, 3);
		}
		else
		{
			code = (code >> 2) - 4;

			*trailing_ones = coeff4_0[code].trailing_ones;
			*total_coff = coeff4_0[code].total_coeff;
			eg_read_skip(bs, 6);
		}
	}
	else
	{
		*trailing_ones = coeff4_1[code].trailing_ones;
		*total_coff = coeff4_1[code].total_coeff;
		eg_read_skip(bs, coeff4_1[code].len);
	}
}

/* nC >= 8 */
void T264dec_mb_read_coff_token_t3(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff)
{
	int32_t code;

	code = eg_read_direct(bs, 6);

	*trailing_ones = coeff3_0[code].trailing_ones;
	*total_coff = coeff3_0[code].total_coeff;
}

/* 8 > nC >= 4 */
void T264dec_mb_read_coff_token_t2(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff)
{
	int32_t code;
	const vlc_coeff_token_t* table;

	code = eg_show(bs, 10);
	if (code >= 512)
	{
		table = coeff2_0;
		code = (code >> 6) - 8;
	}
	else if (code >= 256)
	{
		table = coeff2_1;
		code = (code >> 5) - 8;
	}
	else if (code >= 128)
	{
		table = coeff2_2;
		code = (code >> 4) - 8;
	}
	else if (code >= 64)
	{
		table = coeff2_3;
		code = (code >> 3) - 8;
	}
	else if (code >= 32)
	{
		table = coeff2_4;
		code = (code >> 2) - 8;
	}
	else if (code >= 16)
	{
		table = coeff2_5;
		code = (code >> 1) - 8;
	}
	else
	{
		table = coeff2_6;
	}

	*trailing_ones = table[code].trailing_ones;
	*total_coff = table[code].total_coeff;
	eg_read_skip(bs, table[code].len);
}

/* 4 > nC >= 2 */
void
T264dec_mb_read_coff_token_t1(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff)
{
	int32_t code;
	const vlc_coeff_token_t* table;

	code = eg_show(bs, 14);
	if (code >= 4096)
	{
		table = coeff1_0;
		code = (code >> 10) - 4;
	}
	else if (code >= 1024)
	{
		table = coeff1_1;
		code = (code >> 8) - 4;
	}
	else if (code >= 128)
	{
		table = coeff1_2;
		code = (code >> 5) - 4;
	}
	else if (code >= 64)
	{
		table = coeff1_3;
		code = (code >> 3) - 8;
	}
	else if (code >= 32)
	{
		table = coeff1_4;
		code = (code >> 2) - 8;
	}
	else if (code >= 16)
	{
		table = coeff1_5;
		code = (code >> 1) - 8;
	}
	else
	{
		table = coeff1_6;
	}

	*trailing_ones = table[code].trailing_ones;
	*total_coff = table[code].total_coeff;
	eg_read_skip(bs, table[code].len);
}

/* 2 > nC >= 0 */
void T264dec_mb_read_coff_token_t0(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff)
{
	int32_t code;
	const vlc_coeff_token_t* table;

	code = eg_show(bs, 16);
	if (code >= 8192)
	{
		table = coeff0_5;
		code >>= 13;
	}
	else if (code >= 4096)
	{
		table = coeff0_4;
		code = (code >> 10) - 4;
	}
	else if (code >= 1024)
	{
		table = coeff0_3;
		code = (code >> 8) - 4;
	}
	else if (code >= 128)
	{
		table = coeff0_2;
		code = (code >> 5) - 4;
	}
	else if (code >= 64)
	{
		table = coeff0_1;
		code = (code >> 3) - 8;
	}
	else
	{
		table = coeff0_0;
	}

	*trailing_ones = table[code].trailing_ones;
	*total_coff = table[code].total_coeff;
	eg_read_skip(bs, table[code].len);
}

uint8_t T264dec_mb_read_level_prefix(Bitstream *bs)
{
	uint8_t prefix;
	int32_t code;

	code = eg_show(bs, 16);
	if (code >= 4096)
	{
		prefix = prefix_table0[code >> 12];
	}
	else if (code >= 256)
	{
		prefix = prefix_table1[code >> 8];
	}
	else if (code >= 16)
	{
		prefix = prefix_table2[code >> 4];
	}
	else
	{
		prefix = prefix_table3[code];
	}

	eg_read_skip(bs, prefix + 1);

	return prefix;
}

uint8_t T264dec_mb_read_total_zero1(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 9);
	if (code >= 32)
	{
		code >>= 4;
		total_zero = total_zero_table1_1[code].num;
		eg_read_skip(bs, total_zero_table1_1[code].len);
	}
	else
	{
		total_zero = total_zero_table1_0[code].num;
		eg_read_skip(bs, total_zero_table1_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero2(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 2;
		total_zero = total_zero_table2_1[code].num;
		eg_read_skip(bs, total_zero_table2_1[code].len);
	}
	else
	{
		total_zero = total_zero_table2_0[code].num;
		eg_read_skip(bs, total_zero_table2_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero3(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 2;
		total_zero = total_zero_table3_1[code].num;
		eg_read_skip(bs, total_zero_table3_1[code].len);
	}
	else
	{
		total_zero = total_zero_table3_0[code].num;
		eg_read_skip(bs, total_zero_table3_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero6(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 3;
		total_zero = total_zero_table6_1[code].num;
		eg_read_skip(bs, total_zero_table6_1[code].len);
	}
	else
	{
		total_zero = total_zero_table6_0[code].num;
		eg_read_skip(bs, total_zero_table6_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero7(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 3;
		total_zero = total_zero_table7_1[code].num;
		eg_read_skip(bs, total_zero_table7_1[code].len);
	}
	else
	{
		total_zero = total_zero_table7_0[code].num;
		eg_read_skip(bs, total_zero_table7_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero8(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 3;
		total_zero = total_zero_table8_1[code].num;
		eg_read_skip(bs, total_zero_table8_1[code].len);
	}
	else
	{
		total_zero = total_zero_table8_0[code].num;
		eg_read_skip(bs, total_zero_table8_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero9(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 6);
	if (code >= 8)
	{
		code >>= 3;
		total_zero = total_zero_table9_1[code].num;
		eg_read_skip(bs, total_zero_table9_1[code].len);
	}
	else
	{
		total_zero = total_zero_table9_0[code].num;
		eg_read_skip(bs, total_zero_table9_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero4(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 5);
	if (code >= 16)
	{
		code = (code >> 2) - 4;
		total_zero = total_zero_table4_1[code].num;
		eg_read_skip(bs, total_zero_table4_1[code].len);
	}
	else
	{
		total_zero = total_zero_table4_0[code].num;
		eg_read_skip(bs, total_zero_table4_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero5(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 5);
	if (code >= 16)
	{
		code = (code >> 2) - 4;
		total_zero = total_zero_table5_1[code].num;
		eg_read_skip(bs, total_zero_table5_1[code].len);
	}
	else
	{
		total_zero = total_zero_table5_0[code].num;
		eg_read_skip(bs, total_zero_table5_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero10(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 5);
	if (code >= 4)
	{
		code >>= 2;
		total_zero = total_zero_table10_1[code].num;
		eg_read_skip(bs, total_zero_table10_1[code].len);
	}
	else
	{
		total_zero = total_zero_table10_0[code].num;
		eg_read_skip(bs, total_zero_table10_0[code].len);
	}

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero11(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 4);
	total_zero = total_zero_table11_0[code].num;
	eg_read_skip(bs, total_zero_table11_0[code].len);

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero12(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 4);
	total_zero = total_zero_table12_0[code].num;
	eg_read_skip(bs, total_zero_table12_0[code].len);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero13(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 3);
	total_zero = total_zero_table13_0[code].num;
	eg_read_skip(bs, total_zero_table13_0[code].len);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero14(Bitstream *bs)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 2);
	total_zero = total_zero_table14_0[code].num;
	eg_read_skip(bs, total_zero_table14_0[code].len);

	return total_zero;
}

uint8_t T264dec_mb_read_total_zero15(Bitstream *bs)
{
	return eg_read_direct1(bs);
}

uint8_t T264dec_mb_read_total_zero_chroma(Bitstream *bs, uint8_t total_coeff)
{
	uint8_t total_zero;
	int32_t code;

	code = eg_show(bs, 3);
	total_zero = total_zero_table_chroma[total_coeff - 1][code].num;
	eg_read_skip(bs, total_zero_table_chroma[total_coeff - 1][code].len);

	assert(total_zero != 255);

	return total_zero;
}

uint8_t T264dec_mb_read_run_before(Bitstream *bs, uint8_t zero_left)
{
	int32_t code;
	uint8_t run_before;

	assert(zero_left != 255);

	code = eg_show(bs, 3);
	if (zero_left <= 6)
	{
		run_before = run_before_table_0[zero_left - 1][code].num;
		eg_read_skip(bs, run_before_table_0[zero_left - 1][code].len);
	}
	else
	{
		eg_read_skip(bs, 3);
		if (code > 0)
		{
			run_before = run_before_table_0[6][code].num;
		}
		else
		{
			code = eg_show(bs, 4);
			if (code > 0)
			{
				run_before = run_before_table_1[code];
				eg_read_skip(bs, run_before - 6);
			}
			else
			{
				eg_read_skip(bs, 4);
				code = eg_show(bs, 4);
				run_before = run_before_table_2[code];
				eg_read_skip(bs, run_before - 10);
			}
		}
	}

	assert(run_before <= 14);

	return run_before;
}

int block_residual_read_cavlc(Bitstream *bs, int32_t idx, int16_t* z, int32_t count, int pred)
{
	int ret = 0;

	uint8_t trailing_ones, total_coeff;
	int32_t i, j;
	int32_t zero_left = 0;
	int16_t level[16];
	uint8_t run[16];
//	int32_t x, y;

	if(idx == BLOCK_INDEX_CHROMA_DC)
	{
		T264dec_mb_read_coff_token_t4(bs, &trailing_ones, &total_coeff);
	}
	else
	{
		/* T264_mb_predict_non_zero_code return 0 <-> (16+16+1)>>1 = 16 */
		int32_t nC = 0;
		typedef void (*T264dec_mb_read_coff_token_t)(Bitstream *bs, uint8_t* trailing_ones, uint8_t* total_coff);
		static const T264dec_mb_read_coff_token_t read_coeff[17] = 
		{
			T264dec_mb_read_coff_token_t0, T264dec_mb_read_coff_token_t0,
			T264dec_mb_read_coff_token_t1, T264dec_mb_read_coff_token_t1,
			T264dec_mb_read_coff_token_t2, T264dec_mb_read_coff_token_t2,
			T264dec_mb_read_coff_token_t2, T264dec_mb_read_coff_token_t2,
			T264dec_mb_read_coff_token_t3, T264dec_mb_read_coff_token_t3,
			T264dec_mb_read_coff_token_t3, T264dec_mb_read_coff_token_t3,
			T264dec_mb_read_coff_token_t3, T264dec_mb_read_coff_token_t3,
			T264dec_mb_read_coff_token_t3, T264dec_mb_read_coff_token_t3,
			T264dec_mb_read_coff_token_t3
		};

		if(idx == BLOCK_INDEX_LUMA_DC)
		{
			nC = pred;

			read_coeff[nC](bs, &trailing_ones, &total_coeff);
		}
		else
		{
			nC = pred;

			read_coeff[nC](bs, &trailing_ones, &total_coeff);

			ret = total_coeff;

			assert(total_coeff != 255);
			assert(trailing_ones != 255);

		}
	}

	if (total_coeff > 0)
	{
		uint8_t suffix_length = 0;
		int32_t level_code;

		if (total_coeff > 10 && trailing_ones < 3)
			suffix_length = 1;

		for(i = 0 ; i < trailing_ones ; i ++)
		{
			level[i] = 1 - 2 * eg_read_direct1(bs);
		}

		for( ; i < total_coeff ; i ++)
		{
			uint32_t level_suffixsize;
			uint32_t level_suffix;
			uint8_t level_prefix = T264dec_mb_read_level_prefix(bs);

			level_suffixsize = suffix_length;
			if (suffix_length == 0 && level_prefix == 14)
				level_suffixsize = 4;
			else if (level_prefix == 15)
				level_suffixsize = 12;
			if (level_suffixsize > 0)
				level_suffix = eg_read_direct(bs, level_suffixsize);
			else
				level_suffix = 0;
			level_code = (level_prefix << suffix_length) + level_suffix;
			if (level_prefix == 15 && suffix_length == 0)
			{
				level_code += 15;
			}
			if (i == trailing_ones && trailing_ones < 3)
			{
				level_code += 2;
			}
			if (level_code % 2 == 0)
			{
				level[i] = (level_code + 2) >> 1;
			}
			else
			{
				level[i] = (-level_code - 1) >> 1;
			}

			if (suffix_length == 0)
				suffix_length = 1;

			if (ABS(level[i]) > (3 << (suffix_length - 1)) &&
				suffix_length < 6)
			{
				suffix_length ++;
			}
		}

		if (total_coeff < count)
		{
			typedef uint8_t (*T264dec_mb_read_total_zero_t)(Bitstream *);
			static T264dec_mb_read_total_zero_t total_zero_f[] =
			{
				T264dec_mb_read_total_zero1, T264dec_mb_read_total_zero2, T264dec_mb_read_total_zero3, T264dec_mb_read_total_zero4,
				T264dec_mb_read_total_zero5, T264dec_mb_read_total_zero6, T264dec_mb_read_total_zero7, T264dec_mb_read_total_zero8,
				T264dec_mb_read_total_zero9, T264dec_mb_read_total_zero10, T264dec_mb_read_total_zero11, T264dec_mb_read_total_zero12,
				T264dec_mb_read_total_zero13, T264dec_mb_read_total_zero14, T264dec_mb_read_total_zero15
			};

			if(idx != BLOCK_INDEX_CHROMA_DC)
				zero_left = total_zero_f[total_coeff - 1](bs);
			else
				zero_left = T264dec_mb_read_total_zero_chroma(bs, total_coeff);
		}

		for(i = 0 ; i < total_coeff - 1 ; i ++)
		{
			if (zero_left > 0)
			{
				run[i] = T264dec_mb_read_run_before(bs, zero_left);
			}
			else
			{
				run[i] = 0;
			}
			zero_left -= run[i];
		}

		run[total_coeff - 1] = zero_left;

		j = -1;
		for(i = total_coeff - 1 ; i >= 0 ; i --)
		{
			j +=run[i] + 1;
			z[j] = level[i];
		}
	}
	return ret;
}

void macroblock_read_cavlc_uv(Bitstream *bs, pmacroblockInfo info, cavlc_ref_list *ref_list)
{
	int i;
	if (info->cbp_uv < 2)
	{
		memset(info->dct_uv_h[0], 0, sizeof(info->dct_uv_h[0][0])*4);
		memset(info->dct_uv_h[1], 0, sizeof(info->dct_uv_h[1][0])*4);
		for(i = 0; i < 8 ; ++i)
		{
			memset(info->dct_uv_z[i>>2][i&3], 0, 16);
			set_chroma_pred(ref_list, i, 0);
		}
	}
	if (info->cbp_uv != 0)
	{
		block_residual_read_cavlc(bs, BLOCK_INDEX_CHROMA_DC, info->dct_uv_h[0], 4, 0);
		block_residual_read_cavlc(bs, BLOCK_INDEX_CHROMA_DC, info->dct_uv_h[1], 4, 0);
		if (info->cbp_uv & 0x2)
		{
			for(i = 0; i < 8 ; ++i)
			{
				int nC = get_chroma_pred(ref_list, i);
				nC = block_residual_read_cavlc(bs, 16 + i, info->dct_uv_z[i>>2][i&3]+1, 15, nC);
				set_chroma_pred(ref_list, i, nC);
			}
		}
		for (i = 0; i < 8; ++i)
			info->dct_uv_z[i>>2][i&3][0] = info->dct_uv_h[i>>2][i&3];
	}
}

