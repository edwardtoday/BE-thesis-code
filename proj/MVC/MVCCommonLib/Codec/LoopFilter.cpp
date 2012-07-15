#include <stdlib.h>
#include "LoopFilter.h"

const uint8 filterAlpha[52] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  4,  4,  5,  6,
	7,  8,  9,  10, 12, 13, 15, 17, 20, 22,
	25, 28, 32, 36, 40, 45, 50, 56, 63, 71,
	80, 90, 101,113,127,144,162,182,203,226,
	255,255
};

const uint8 filterBeta[52]  =
{
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  2,  2,  2,  3,
	3,  3,  3,  4,  4,  4,  6,  6,  7,  7,
	8,  8,  9,  9, 10, 10, 11, 11, 12, 12,
	13, 13, 14, 14, 15, 15, 16, 16, 17, 17,
	18, 18
};

const uint8 filterClip[52][5] =
{
	{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0}, {0, 0, 0, 1, 1}, {0, 0, 0, 1, 1}, {0, 0, 0, 1, 1},
	{0, 0, 0, 1, 1}, {0, 0, 1, 1, 1}, {0, 0, 1, 1, 1}, {0, 1, 1, 1, 1},
	{0, 1, 1, 1, 1}, {0, 1, 1, 1, 1}, {0, 1, 1, 1, 1}, {0, 1, 1, 2, 2},
	{0, 1, 1, 2, 2}, {0, 1, 1, 2, 2}, {0, 1, 1, 2, 2}, {0, 1, 2, 3, 3},
	{0, 1, 2, 3, 3}, {0, 2, 2, 3, 3}, {0, 2, 2, 4, 4}, {0, 2, 3, 4, 4},
	{0, 2, 3, 4, 4}, {0, 3, 3, 5, 5}, {0, 3, 4, 6, 6}, {0, 3, 4, 6, 6},
	{0, 4, 5, 7, 7}, {0, 4, 5, 8, 8}, {0, 4, 6, 9, 9}, {0, 5, 7,10,10},
	{0, 6, 8,11,11}, {0, 6, 8,13,13}, {0, 7,10,14,14}, {0, 8,11,16,16},
	{0, 9,12,18,18}, {0,10,13,20,20}, {0,11,15,23,23}, {0,13,17,25,25}
} ;

typedef uint8 TYPE_AAAUCBS[2][4][4];

void Filter(uint8 *pixels, int stride, int idxA, int idxB, uint8 ucBs, bool isLuma)
{
	const int iAlpha = filterAlpha[ idxA ];

	int P0 = pixels[-stride];
	int Q0 = pixels[      0];

	int iDelta = Q0 - P0;
	int iAbsDelta  = abs(iDelta);

	AOF_DBG( ucBs );

	if (!(iAbsDelta<iAlpha))
		return ;

	const int iBeta = filterBeta[ idxB ];

	int P1  = pixels[-2*stride];
	int Q1  = pixels[   stride];

	if (!((abs(P0 - P1) < iBeta) && (abs(Q0 - Q1) < iBeta)))
		return ;

	if( ucBs < 4 )
	{
		int C0 = filterClip[ idxA ][ucBs];

		if( isLuma )
		{
			int P2 = pixels[-3*stride] ;
			int Q2 = pixels[ 2*stride] ;
			int aq = (( abs( Q2 - Q0 ) < iBeta ) ? 1 : 0 );
			int ap = (( abs( P2 - P0 ) < iBeta ) ? 1 : 0 );

			if( ap )
			{
				pixels[-2*stride] = P1 + ClipMinMax((P2 + ((P0 + Q0 + 1)>>1) - (P1<<1)) >> 1, -C0, C0 );
			}

			if( aq  )
			{
				pixels[   stride] = Q1 + ClipMinMax((Q2 + ((P0 + Q0 + 1)>>1) - (Q1<<1)) >> 1, -C0, C0 );
			}

			C0 += ap + aq -1;
		}

		C0++;
		int iDiff      = ClipMinMax(((iDelta << 2) + (P1 - Q1) + 4) >> 3, -C0, C0 ) ;
		pixels[-stride] = CLIP1( P0 + iDiff );
		pixels[      0] = CLIP1( Q0 - iDiff );
		return;
	}


	if( ! isLuma )
	{
		pixels[        0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
		pixels[  -stride] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
	}
	else
	{
		int P2 = pixels[-3*stride] ;
		int Q2 = pixels[ 2*stride] ;
		bool bEnable  = (iAbsDelta < ((iAlpha >> 2) + 2));
		bool aq       = bEnable & ( abs( Q2 - Q0 ) < iBeta );
		bool ap       = bEnable & ( abs( P2 - P0 ) < iBeta );
		int PQ0 = P0 + Q0;

		if( aq )
		{
			pixels[        0] = (P1 + ((Q1 + PQ0) << 1) +  Q2 + 4) >> 3;
			pixels[   stride] = (PQ0 +Q1 + Q2 + 2) >> 2;
			pixels[ 2*stride] = (((pixels[ 3*stride] + Q2) <<1) + Q2 + Q1 + PQ0 + 4) >> 3;
		}
		else
		{
			pixels[        0] = ((Q1 << 1) + Q0 + P1 + 2) >> 2;
		}

		if( ap )
		{
			pixels[  -stride] = (Q1 + ((P1 + PQ0) << 1) +  P2 + 4) >> 3;
			pixels[-2*stride] = (PQ0 +P1 + P2 + 2) >> 2;
			pixels[-3*stride] = (((pixels[-4*stride] + P2) <<1) + pixels[-3*stride] + P1 + PQ0 + 4) >> 3;
		}
		else
		{
			pixels[  -stride] = ((P1 << 1) + P0 + Q1 + 2) >> 2;
		}
	}
}

void LumaVerFilter(CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx, TYPE_AAAUCBS m_aaaucBs)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	uint8*  puint8Lum = cpi->lumaStart + mbY * 16 * cpi->Size_Width + mbX * 16;
	int   iStride = cpi->Size_Width;
	int   iCurrQp = cpi->baseQP + mlist[mbIdx].qp_delta;
//	int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();

	//===== filtering of left macroblock edge =====
	{
		int iLeftQp = (mbX?(cpi->baseQP+mlist[mbIdx-1].qp_delta):0); //rcMbDataAccess.getMbDataLeft().getQpLF();
		int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
		int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iQp, 0, 51);
		int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iQp, 0, 51);

		for( int yBlk = 0; yBlk < 4; yBlk++)
		{
			const uint8 ucBs = m_aaaucBs[0][0][yBlk];
			if( 0 != ucBs )
			{
				Filter( puint8Lum,           1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
			}
			puint8Lum += 4*iStride;
		}
		puint8Lum -= 16*iStride-4;
	}


	int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iCurrQp, 0, 51);
	int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iCurrQp, 0, 51);

	for( int xBlk = 1; xBlk < 4; xBlk++)
	{
		for( int yBlk = 0; yBlk < 4; yBlk++)
		{
			const uint8 ucBs = m_aaaucBs[0][xBlk][yBlk];
			if( 0 != ucBs )
			{
				Filter( puint8Lum,           1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+  iStride, 1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+2*iStride, 1, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+3*iStride, 1, iIndexA, iIndexB, ucBs, true );
			}
			puint8Lum += 4*iStride;
		}
		puint8Lum -= 16*iStride-4;
	}
}

void LumaHorFilter(CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx, TYPE_AAAUCBS m_aaaucBs)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	uint8*  puint8Lum = cpi->lumaStart + mbY * 16 * cpi->Size_Width + mbX * 16;
	int   iCurrQp = cpi->baseQP + mlist[mbIdx].qp_delta;
//	int   iCurrQp = rcMbDataAccess.getMbDataCurr().getQpLF();
	int   iStride = cpi->Size_Width;

	//===== filtering of upper macroblock edge =====
	{
		//-----  any other combination than curr = FRM, above = FLD  -----
		int iAboveQp  = (mbY?(cpi->baseQP+mlist[mbIdx-mbW].qp_delta):0); //rcMbDataAccess.getMbDataAbove().getQpLF();
		int iQp       = ( iAboveQp + iCurrQp + 1) >> 1;
		int iIndexA   = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iQp, 0, 51);
		int iIndexB   = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iQp, 0, 51);

		for( int xBlk = 0; xBlk < 4; xBlk++)
		{
			const uint8 ucBs = m_aaaucBs[1][xBlk][0];
			if( 0 != ucBs )
			{
				Filter( puint8Lum,   iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+1, iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+2, iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+3, iStride, iIndexA, iIndexB, ucBs, true );
			}
			puint8Lum += 4;
		}
		puint8Lum -= 16;
	}

	puint8Lum += 4*iStride;
	int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iCurrQp, 0, 51);
	int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iCurrQp, 0, 51);

	for( int yBlk = 1; yBlk < 4; yBlk++)
	{
		for( int xBlk = 0; xBlk < 4; xBlk++)
		{
			const uint8 ucBs = m_aaaucBs[1][xBlk][yBlk];
			if( 0 != ucBs )
			{
				Filter( puint8Lum,   iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+1, iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+2, iStride, iIndexA, iIndexB, ucBs, true );
				Filter( puint8Lum+3, iStride, iIndexA, iIndexB, ucBs, true );
			}
			puint8Lum += 4;
		}
		puint8Lum += 4*iStride - 16;
	}
}

void ChromaHorFilter(CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx, TYPE_AAAUCBS m_aaaucBs)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	uint8*  puint8Cb = cpi->chromaUStart + mbY * 4 * cpi->Size_Width + mbX * 8;
	uint8*  puint8Cr = cpi->chromaVStart + mbY * 4 * cpi->Size_Width + mbX * 8;
	int   iCurrQp = QPLuma2Chroma[cpi->baseQP + mlist[mbIdx].qp_delta];
//	int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
	int   iStride = cpi->Size_Width / 2;

	//===== filtering of upper macroblock edge =====
	{
		//-----  any other combination than curr = FRM, above = FLD  -----
		int iAboveQp  = (mbY?QPLuma2Chroma[(cpi->baseQP+mlist[mbIdx-mbW].qp_delta)]:0); //rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataAbove().getQpLF() );
		int iQp       = ( iAboveQp + iCurrQp + 1) >> 1;
		int iIndexA   = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iQp, 0, 51);
		int iIndexB   = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iQp, 0, 51);

		for( int xBlk = 0; xBlk < 4; xBlk++)
		{
			const uint8 ucBs = m_aaaucBs[1][xBlk][0];
			if( 0 != ucBs )
			{
				Filter( puint8Cb,   iStride, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cb+1, iStride, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cr,   iStride, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cr+1, iStride, iIndexA, iIndexB, ucBs, false );
			}
			puint8Cb += 2;
			puint8Cr += 2;
		}
		puint8Cb -= 8;
		puint8Cr -= 8;
	}

	puint8Cb += 4*iStride;
	puint8Cr += 4*iStride;

	// now we filter the remaining edge
	int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iCurrQp, 0, 51);
	int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iCurrQp, 0, 51);

	for( int xBlk = 0; xBlk < 4; xBlk++)
	{
		const uint8 ucBs = m_aaaucBs[1][xBlk][2];
		if( 0 != ucBs )
		{
			Filter( puint8Cb,   iStride, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cb+1, iStride, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cr,   iStride, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cr+1, iStride, iIndexA, iIndexB, ucBs, false );
		}
		puint8Cb += 2;
		puint8Cr += 2;
	}
}

void ChromaVerFilter(CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx, TYPE_AAAUCBS m_aaaucBs)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	uint8*  puint8Cb = cpi->chromaUStart + mbY * 4 * cpi->Size_Width + mbX * 8;
	uint8*  puint8Cr = cpi->chromaVStart + mbY * 4 * cpi->Size_Width + mbX * 8;
	int   iCurrQp = QPLuma2Chroma[cpi->baseQP + mlist[mbIdx].qp_delta];
//	int   iCurrQp = rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataCurr().getQpLF() );
	int   iStride = cpi->Size_Width / 2;

	//===== filtering of left macroblock edge =====
	{
		//-----  curr == FRM && left == FRM  or  curr == FLD && left == FLD  -----
		int iLeftQp = (mbX?QPLuma2Chroma[(cpi->baseQP+mlist[mbIdx-1].qp_delta)]:0); //rcMbDataAccess.getSH().getChromaQp( rcMbDataAccess.getMbDataLeft().getQpLF() );
		int iQp     = ( iLeftQp + iCurrQp + 1) >> 1;
		int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iQp, 0, 51);
		int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iQp, 0, 51);

		for( int yBlk = 0; yBlk < 4; yBlk++)
		{
			const uint8 ucBs = m_aaaucBs[0][0][yBlk];
			if( 0 != ucBs )
			{
				Filter( puint8Cb,         1, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cb+iStride, 1, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cr,         1, iIndexA, iIndexB, ucBs, false );
				Filter( puint8Cr+iStride, 1, iIndexA, iIndexB, ucBs, false );
			}
			puint8Cb += 2*iStride;
			puint8Cr += 2*iStride;
		}
	}

	puint8Cb -= 8*iStride-4;
	puint8Cr -= 8*iStride-4;

	int iIndexA = ClipMinMax( /*rcDFP.getSliceAlphaC0Offset()*/0 + iCurrQp, 0, 51);
	int iIndexB = ClipMinMax( /*rcDFP.getSliceBetaOffset()*/0    + iCurrQp, 0, 51);
	for( int yBlk = 0; yBlk < 4; yBlk++)
	{
		const uint8 ucBs = m_aaaucBs[0][2][yBlk];
		if( 0 != ucBs )
		{
			Filter( puint8Cb,         1, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cb+iStride, 1, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cr,         1, iIndexA, iIndexB, ucBs, false );
			Filter( puint8Cr+iStride, 1, iIndexA, iIndexB, ucBs, false );
		}
		puint8Cb += 2*iStride;
		puint8Cr += 2*iStride;
	}
}

int CheckMvDataP(macroblockInfo *mb1, int idx1,
				 macroblockInfo *mb2, int idx2)
{
	if (veryNearS(mb1->refs[0][idx1][0], mb2->refs[0][idx2][0]))
		return 0;
	return 1;
	/*
	if (mb1->refs[0][idx1][0].refno != mb2->refs[0][idx2][0].refno)
		return 1;
	if (abs(mb1->refs[0][idx1][0].x - mb2->refs[0][idx2][0].x) >= 4)
		return 1;
	if (abs(mb1->refs[0][idx1][0].y - mb2->refs[0][idx2][0].y) >= 4)
		return 1;
	return 0;
	*/
}

int CheckMvDataB(CodingPictInfo *cpi, 
				 macroblockInfo *mb1, int idx1,
				 macroblockInfo *mb2, int idx2)
{
	if (veryNearD(cpi, mb1->refs[0][idx1][0], mb2->refs[0][idx2][1]) &&
		veryNearD(cpi, mb2->refs[0][idx2][0], mb1->refs[0][idx1][1]))
		return 0;
	if (veryNearS(mb1->refs[0][idx1][0], mb2->refs[0][idx2][0]) &&
		veryNearS(mb1->refs[0][idx1][1], mb2->refs[0][idx2][1]))
		return 0;
	return 1;
	/*
	if (mb1->refs[0][idx1][0].refno != mb2->refs[0][idx2][0].refno)
		return 1;
	if (mb1->refs[0][idx1][0].refno != -1)
	{
		if (abs(mb1->refs[0][idx1][0].x - mb2->refs[0][idx2][0].x) >= 4)
			return 1;
		if (abs(mb1->refs[0][idx1][0].y - mb2->refs[0][idx2][0].y) >= 4)
			return 1;
	}
	if (mb1->refs[0][idx1][1].refno != mb2->refs[0][idx2][1].refno)
		return 1;
	if (mb1->refs[0][idx1][1].refno != -1)
	{
		if (abs(mb1->refs[0][idx1][1].x - mb2->refs[0][idx2][1].x) >= 4)
			return 1;
		if (abs(mb1->refs[0][idx1][1].y - mb2->refs[0][idx2][1].y) >= 4)
			return 1;
	}
	return 0;
	*/
}

int GetVerFilterStrength( CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx,
												int              idx)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	macroblockInfo *mcur = mlist + mbIdx;
	macroblockInfo *mleft = mlist + mbIdx - 1;
	if( idx&3 )
	{
		// this is a edge inside of a macroblock
		ROTRS( mlist[mbIdx].mbtype==2, 3 );

		if (isCoded(mcur, idx))
			return 2;
		if (isCoded(mcur, idx-1))
			return 2;
		if( mcur->mbtype==0 )
		{
			return CheckMvDataP( mcur, idx, mcur, idx - 1);
		}
		return CheckMvDataB  ( cpi, mcur, idx, mcur, idx - 1);
	}

	ROTRS( ! mlist[mbIdx].mbC, 0 );

	ROTRS( mlist[mbIdx].mbtype==2, 4 );
	ROTRS( mlist[mbIdx-1].mbtype==2, 4 );

	if (isCoded(mcur, idx))
		return 2;
	if (isCoded(mleft, idx+3))
		return 2;

	if( mcur->mbtype==0 && mleft->mbtype==0)
	{
		return CheckMvDataP( mcur, idx, mleft, idx + 3);
	}
	return CheckMvDataB  ( cpi, mcur, idx, mleft, idx + 3);
}



int GetHorFilterStrength( CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx,
												int              idx)
{
	int mbW = cpi->Size_Width / 16;
	int mbX = mbIdx % mbW,
		mbY = mbIdx / mbW;
	macroblockInfo *mcur = mlist + mbIdx;
	macroblockInfo *mabove = mlist + mbIdx - mbW;
	if( idx>>2 )
	{
		// internal edge
		ROTRS( mlist[mbIdx].mbtype==2, 3 );

		if (isCoded(mcur, idx))
			return 2;
		if (isCoded(mcur, idx-4))
			return 2;
		if( mcur->mbtype==0 )
			return CheckMvDataP( mcur, idx, mcur, idx - 4);
		return CheckMvDataB  ( cpi, mcur, idx, mcur, idx - 4);
	}

	ROTRS( ! mlist[mbIdx].mbR,                    0 );


	ROTRS( mlist[mbIdx].mbtype==2, 4 );
	ROTRS( mlist[mbIdx-mbW].mbtype==2, 4 );

	if (isCoded(mcur, idx))
		return 2;
	if (isCoded(mabove, idx+12))
		return 2;
	if( mcur->mbtype==0 && mabove->mbtype==0)
	{
		return CheckMvDataP( mcur, idx, mabove, idx + 12);
	}
	return CheckMvDataB  ( cpi, mcur, idx, mabove, idx + 12);
}


void FilterMB(CodingPictInfo *cpi, macroblockInfo *mlist, int mbIdx)
{
	/*
	if (disableLoopFilter)
		return ;
	if ( (m_eLFMode & LFM_NO_INTER_FILTER) && ! rcMbDataAccess.getMbData().isintra())
		return ;
		*/

	TYPE_AAAUCBS m_aaaucBs;
	//uint8 m_aaaucBs[2][4][4];

	Bool b8x8 = (0!=(mlist[mbIdx].transform8x8));//rcMbDataAccess.getMbData().isTransformSize8x8();

	for (int idx = 0; idx < 16; ++idx)
	{
		if( !b8x8 || ( ( (idx&3) & 1 ) == 0 ) )
		{
			m_aaaucBs[0][(idx&3)][(idx>>2)]  = GetVerFilterStrength( cpi, mlist, mbIdx, idx);
		}
		else
		{
			m_aaaucBs[0][(idx&3)][(idx>>2)]  = 0;
		}
		if( !b8x8 || ( ( (idx>>2) & 1 ) == 0 ) )
		{
			m_aaaucBs[1][(idx&3)][(idx>>2)]  = GetHorFilterStrength( cpi, mlist, mbIdx, idx);
		}
		else
		{
			m_aaaucBs[1][(idx&3)][(idx>>2)]  = 0;
		}
	}

	LumaVerFilter(   cpi, mlist, mbIdx, m_aaaucBs);
	LumaHorFilter(   cpi, mlist, mbIdx, m_aaaucBs);
	ChromaVerFilter( cpi, mlist, mbIdx, m_aaaucBs);
	ChromaHorFilter( cpi, mlist, mbIdx, m_aaaucBs);
}
