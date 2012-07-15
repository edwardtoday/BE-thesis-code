#include "vlcPred.h"
#include <iostream>

//int get_luma_pred(cavlc_ref_list *ref_list, int idx)
//{
//	int row = idx / 4, col = idx & 3, nA = ref_list->left[row].coeff, nB = ref_list->top[col].coeff, nC = nA+nB;
//	if (nC & 0xC0)
//		nC = nC & 0x3F;
//	else
//		nC = (nC + 1) >> 1;
//	return nC & 0x3F;
//}

//void set_luma_pred(cavlc_ref_list *ref_list, int idx, int value)
//{
//	int row = idx / 4, col = idx & 3;
//	ref_list->left[row].coeff = ref_list->top[col].coeff = value;
//}

//int get_mode_pred(cavlc_ref_list *ref_list, int idx)
//{
//	int row = idx / 4, col = idx & 3, nA = ref_list->left[row].imode, nB = ref_list->top[col].imode;
//	if ((nA&0xC0)||(nB&0xC0))
//		nA = nB = 2;
//	if (nB < nA)
//		nA = nB;
//	return nA;
//}
//
//void set_mode_pred(cavlc_ref_list *ref_list, int idx, int value)
//{
//	int row = idx / 4, col = idx & 3;
//	ref_list->left[row].imode = ref_list->top[col].imode = value;
//}
//
//int get_chroma_pred(cavlc_ref_list *ref_list, int index)
//{
//	int row = index >> 1, col = ((index & 4) >> 1) | (index & 1), nA = ref_list->left[row].coeffC, nB = ref_list->top[col].coeffC, nC = nA+nB;
//	if (nC & 0xC0)
//		nC = nC & 0x3F;
//	else
//		nC = (nC + 1) >> 1;
//	return nC & 0x3F;
//}
//
//void set_chroma_pred(cavlc_ref_list *ref_list, int index, int value)
//{
//	int row = index >> 1, col = ((index & 4) >> 1) | (index & 1);
//	ref_list->left[row].coeffC = ref_list->top[col].coeffC = value;
//}
//
//void swap(int &a, int &b)
//{
//	int c = a;
//	a = b; 
//	b = c;
//}

#define MEDIAN(a,b,c)  ((a)>(b)?(a)>(c)?(b)>(c)?(b):(c):(a):(b)>(c)?(a)>(c)?(a):(c):(b))
#define min(a,b) ((a)>(b)?(b):(a))

void getRefXYP(cavlc_ref_list *ref_list, int r, int c1, int c2, refVector &ref, bool allowAboveRight, int type, int prefered)
{
	getRefXY(ref_list, 0, r, c1, c2, ref, allowAboveRight, type, prefered);
	if (ref.refno < 0)
	{
		ref.x = ref.y = ref.refno = 0;
	}
}

void getRefXYB(cavlc_ref_list *ref_list, int r, int c1, int c2, refVector *ref, bool allowAboveRight, int type, int prefered0, int prefered1)
{
	getRefXY(ref_list, 0, r, c1, c2, ref[0], allowAboveRight, type, prefered0);
	getRefXY(ref_list, 1, r, c1, c2, ref[1], allowAboveRight, type, prefered1);
}

void getRefXY(cavlc_ref_list *ref_list, int listId, int r, int c1, int c2, refVector &ref, bool allowAboveRight, int type, int prefered)
{
	int x1 = ref_list->left[r].mv_x[listId],
		x2 = ref_list->top[c1].mv_x[listId],
		x3 = ref_list->top[c2].mv_x[listId];
	int y1 = ref_list->left[r].mv_y[listId],
		y2 = ref_list->top[c1].mv_y[listId],
		y3 = ref_list->top[c2].mv_y[listId];
	int n1 = ref_list->left[r].refid[listId],
		n2 = ref_list->top[c1].refid[listId],
		n3 = ref_list->top[c2].refid[listId];
	if (!allowAboveRight || x3 == 0x4000)
	{
		x3 = ref.x;
		y3 = ref.y;
		n3 = ref.refno;
	}
	bool o1 = (x1!=0x2000) && (x1!=0x4000),
		o2 = (x2!=0x2000) && (x2!=0x4000),
		o3 = (x3!=0x2000) && (x3!=0x4000);
	if (prefered>=0)
	{
		ref.refno = prefered;
	}
	else
	{
		ref.refno = -1;
		if (o1 && (uint8_t)ref_list->left[r].refid[listId] < (uint8_t)ref.refno)
			ref.refno = ref_list->left[r].refid[listId];
		if (o2 && (uint8_t)ref_list->top[c1].refid[listId] < (uint8_t)ref.refno)
			ref.refno = ref_list->top[c1].refid[listId];
		if (o3 && (uint8_t)n3 < (uint8_t)ref.refno)
			ref.refno = n3;
	}
	if (ref.refno < 0)
	{
		ref.x = 0x2000;
		ref.y = 0x2000;
	}
	else if (o1 && (type==1 && n1==ref.refno /*|| !o2 && !o3*/ || n1==ref.refno&&n2!=ref.refno&&n3!=ref.refno))
	{
		ref.x = x1;
		ref.y = y1;
	}
	else if (o2 && (type==2 && n2==ref.refno /*|| !o1 && !o3*/ || n2==ref.refno&&n1!=ref.refno&&n3!=ref.refno))
	{
		ref.x = x2;
		ref.y = y2;
	}
	else if (o3 && (type==3 && n3==ref.refno /*|| !o1 && !o2*/ || n3==ref.refno&&n2!=ref.refno&&n1!=ref.refno))
	{
		ref.x = x3;
		ref.y = y3;
	}
	else if (x2==0x4000 && x3==0x4000)
	{
		ref.x = (x1&0x0FFF)-(x1&0x1000);
		ref.y = (y1&0x0FFF)-(y1&0x1000);
	}
	else
	{
		ref.x = MEDIAN((x1&0x0FFF)-(x1&0x1000), (x2&0x0FFF)-(x2&0x1000), (x3&0x0FFF)-(x3&0x1000));
		ref.y = MEDIAN((y1&0x0FFF)-(y1&0x1000), (y2&0x0FFF)-(y2&0x1000), (y3&0x0FFF)-(y3&0x1000));
	}
}

void getRefXY_SKIP(cavlc_ref_list *ref_list, int listId, int r, int c1, int c2, refVector &ref, bool allowAboveRight, int type)
{
	int x1 = ref_list->left[r].mv_x[listId],
		x2 = ref_list->top[c1].mv_x[listId];
	int y1 = ref_list->left[r].mv_y[listId],
		y2 = ref_list->top[c1].mv_y[listId];
	if ((x1==0x4000) || (x2==0x4000) || 
		x1==0 && y1==0 && ref_list->left[r].refid[listId]==0 ||
		x2==0 && y2==0 && ref_list->top[c1].refid[listId]==0)
	{
		ref.refno = 0;
		ref.x = 0;
		ref.y = 0;
		return ;
	}
	getRefXY(ref_list, listId, r, c1, c2, ref, allowAboveRight, type);
}

void setRefXY(cavlc_ref_list *ref_list, int r, int c, refVector &ref)
{
	setRefXY(ref_list, 0, r, c, ref);
}

void setRefXY(cavlc_ref_list *ref_list, int listId, int r, int c, refVector &ref)
{
	//const int refno = ref.refno;
	ref_list->left[r].refid[listId] = ref.refno;
	ref_list->top[c].refid[listId] = ref.refno;
	if (ref.refno == -1)
	{
		ref_list->left[r].mv_x[listId] = 0x2000;
		ref_list->left[r].mv_y[listId] = 0x2000;
		ref_list->top[c].mv_x[listId] = 0x2000;
		ref_list->top[c].mv_y[listId] = 0x2000;
	}
	else
	{
		ref_list->left[r].mv_x[listId] = ref.x;
		ref_list->top[c].mv_x[listId] = ref.x;
		ref_list->left[r].mv_y[listId] = ref.y;
		ref_list->top[c].mv_y[listId] = ref.y;
	}
}

//void copyVector(refVector &vec, cavlc_ref &ref)
//{
//	vec.refno = ref.refid[0];
//	vec.x = ref.mv_x[0];
//	vec.y = ref.mv_y[0];
//}
//
//void copyVector2(refVector &vec, cavlc_ref &ref)
//{
//	vec.refno = ref.refid[1];
//	vec.x = ref.mv_x[1];
//	vec.y = ref.mv_y[1];
//}

void getRefXYDirectB(cavlc_ref_list *ref_list, refVector *col_refs, pmacroblockInfo info, int r, int c1, int c2, refVector *ref)
{

	Bool          bDirectZeroPred   = false;
	Bool          bAllColNonZero    = false;
	Bool          bColZeroFlagBlk0  = false;
	Bool          bColZeroFlagBlk1  = false;
	Bool          bColZeroFlagBlk2  = false;
	Bool          bColZeroFlagBlk3  = false;

	//===== get reference indices and spatially predicted motion vectors =====
	int ascRefIdx[2];
	for (int i = 0; i < 2; ++i)
	{
		int listId = i;
		int x1 = ref_list->lefts[0].mv_x[listId],
			x2 = ref_list->tops[0].mv_x[listId],
			x3 = ref_list->tops[4].mv_x[listId];
		int y1 = ref_list->lefts[0].mv_y[listId],
			y2 = ref_list->tops[0].mv_y[listId],
			y3 = ref_list->tops[4].mv_y[listId];
		int n1 = ref_list->lefts[0].refid[listId],
			n2 = ref_list->tops[0].refid[listId],
			n3 = ref_list->tops[4].refid[listId];
		if (x3 == 0x4000)
		{
			x3 = ref[listId].x;
			y3 = ref[listId].y;
			n3 = ref[listId].refno;
		}
		ref[listId].refno = -1;
		ref[listId].x = 0x2000;
		ref[listId].y = 0x2000;
		ascRefIdx[listId] = n1;
		if (n2>=0 && ((n2<ascRefIdx[listId]) || ascRefIdx[listId]<0))
			ascRefIdx[listId] = n2;
		if (n3>=0 && ((n3<ascRefIdx[listId]) || ascRefIdx[listId]<0))
			ascRefIdx[listId] = n3;
		if( ascRefIdx[listId] >= 0 )
		{
			if (n1==ascRefIdx[listId] && n2!=ascRefIdx[listId] && n3!=ascRefIdx[listId] ||
				x2==0x4000 && x3==0x4000)
			{
				ref[listId].refno = n1;
				ref[listId].x = x1;
				ref[listId].y = y1;
			}
			else if (n2==ascRefIdx[listId] && n1!=ascRefIdx[listId] && n3!=ascRefIdx[listId] ||
				x1==0x4000 && x3==0x4000)
			{
				ref[listId].refno = n2;
				ref[listId].x = x2;
				ref[listId].y = y2;
			}
			else if (n3==ascRefIdx[listId] && n1!=ascRefIdx[listId] && n2!=ascRefIdx[listId] ||
				x1==0x4000 && x2==0x4000)
			{
				ref[listId].refno = n3;
				ref[listId].x = x3;
				ref[listId].y = y3;
			}
			else
			{
				ref[listId].refno = ascRefIdx[listId];
				ref[listId].x = MEDIAN((x1&0x0FFF)-(x1&0x1000), (x2&0x0FFF)-(x2&0x1000), (x3&0x0FFF)-(x3&0x1000));
				ref[listId].y = MEDIAN((y1&0x0FFF)-(y1&0x1000), (y2&0x0FFF)-(y2&0x1000), (y3&0x0FFF)-(y3&0x1000));
			}
		}
	}

	if( ascRefIdx[0] < 0 && ascRefIdx[1] < 0 )
	{
		ascRefIdx[0] = 0;
		ascRefIdx[1] = 0;
		ref[0].refno = 0;
		ref[1].refno = 0;
		bDirectZeroPred   = true;
		bAllColNonZero    = true;
	}

	//===== check co-located =====
	if( ! bAllColNonZero )
	{
		int   scRefIdxCol;
		refVector      acMvCol[4];

		if( ! bAllColNonZero )
		{
			//if( b8x8 )
			{
				//SParIdx4x4 eSubMbPartIdx = ( eParIdx <= PART_8x8_1 ? ( eParIdx == PART_8x8_0 ? SPART_4x4_0 : SPART_4x4_1 )
				//	: ( eParIdx == PART_8x8_2 ? SPART_4x4_2 : SPART_4x4_3 ) );

				//xGetColocatedMvRefIdx( acMvCol[0], scRefIdxCol, B4x4Idx( eParIdx + eSubMbPartIdx ) );
				// todo:
				int px = r, py = c1;
				if (px) ++px;
				if (py) ++py;
				acMvCol[0] = col_refs[px*8 + py*2];
				if (acMvCol[0].refno<0)
				{
					acMvCol[0] = col_refs[px*8 + py*2 + 1];
				}
				scRefIdxCol = acMvCol[0].refno;
			}
			//else
			//{
				//===== THIS SHALL NEVER BE CALLED FOR INTERLACED SEQUENCES =====
			//	xGetColocatedMvsRefIdxNonInterlaced( acMvCol, scRefIdxCol, eParIdx );
			//}

			bAllColNonZero = ( scRefIdxCol != 0 );
		}

		if( ! bAllColNonZero )
		{
			bColZeroFlagBlk0   = ( abs(acMvCol[0].x) <= 1 && abs(acMvCol[0].y) <= 1 );

			//if( ! b8x8 )
			//{
			//	bColZeroFlagBlk1 = ( acMvCol[1].getAbsHor() <= 1 && acMvCol[1].getAbsVer() <= 1 );
			//	bColZeroFlagBlk2 = ( acMvCol[2].getAbsHor() <= 1 && acMvCol[2].getAbsVer() <= 1 );
			//	bColZeroFlagBlk3 = ( acMvCol[3].getAbsHor() <= 1 && acMvCol[3].getAbsVer() <= 1 );
			//}
		}
	}

	//===== set motion vectors and reference frames =====
	for (int i = 0; i < 2; ++i)
	{
		int listId = i;
		int         scRefIdx          = ascRefIdx[ listId ];

		Bool          bZeroMv;



		//----- set motion vectors -----
		//if( /*b8x8 ||*/ bAllColNonZero || scRefIdx < 0 )
		{
			bZeroMv         = ( bDirectZeroPred || scRefIdx < 0 || ( scRefIdx == 0 && bColZeroFlagBlk0 ) );
			if (bZeroMv)
			{
				ref[listId].x = 0;
				ref[listId].y = 0;
			}
			//const Mv& rcMv  = ( bZeroMv ? Mv::ZeroMv() : acMvPred [ listId ] );
			//rcMbMotionDataLX.setAllMv( rcMv, eParIdx );
		}
		/*
		else
		{
			bZeroMv         = ( scRefIdx == 1 && bColZeroFlagBlk0 );
			const Mv& rcMv0 = ( bZeroMv ? Mv::ZeroMv() : acMvPred [ eListIdx ] );
			rcMbMotionDataLX.setAllMv( rcMv0, eParIdx, SPART_4x4_0 );

			bZeroMv         = ( scRefIdx == 1 && bColZeroFlagBlk1 );
			const Mv& rcMv1 = ( bZeroMv ? Mv::ZeroMv() : acMvPred [ eListIdx ] );
			rcMbMotionDataLX.setAllMv( rcMv1, eParIdx, SPART_4x4_1 );

			bZeroMv         = ( scRefIdx == 1 && bColZeroFlagBlk2 );
			const Mv& rcMv2 = ( bZeroMv ? Mv::ZeroMv() : acMvPred [ eListIdx ] );
			rcMbMotionDataLX.setAllMv( rcMv2, eParIdx, SPART_4x4_2 );

			bZeroMv         = ( scRefIdx == 1 && bColZeroFlagBlk3 );
			const Mv& rcMv3 = ( bZeroMv ? Mv::ZeroMv() : acMvPred [ eListIdx ] );
			rcMbMotionDataLX.setAllMv( rcMv3, eParIdx, SPART_4x4_3 );
		}
		*/


		//----- set reference indices and reference pictures -----
		/*
		rcMbMotionDataLX.setRefIdx ( scRefIdx,  eParIdx );

		if( m_rcSliceHeader.getRefPicList( getMbPicType(), eListIdx ).size() )
		{
			const Frame* pcFrame = ( scRefIdx < 1 ? 0 : m_rcSliceHeader.getRefPic ( scRefIdx, getMbPicType(), eListIdx ).getFrame() );
			rcMbMotionDataLX.setRefPic ( pcFrame,   eParIdx );
		}
		*/
	}

}
