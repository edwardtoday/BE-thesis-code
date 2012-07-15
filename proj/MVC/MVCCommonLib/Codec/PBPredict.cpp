/************************************************************************
 *
 * MVC Codec
 * 
 * History
 * 2008-11-29		Weidong Hu		Tsinghua University		Reorganize
 * 2006-2008 		Weidong Hu		Tsinghua University		Create
 * 
 ************************************************************************/

#include "PBPredict.h"
#include "Codec/Consts4Standard.h"

void macroblockPredGetDataY(PBPredictInfo *pif, uint8_t *dat, int px, int py, int sx, int sy)
{
	for (int i = 0; i < sx; ++i)
	{
		int x = px + i;
		if (x<0) x = 0;
		else if (x >= pif->sizeh) x = pif->sizeh - 1;
		uint8_t *linef = dat + x * pif->stride;
		int dl = 0, sl = py, sr = py + sy;
		if (sl < 0)
		{
			dl -= sl;
			sl = 0;
		}
		if (sr > pif->sizew)
			sr = pif->sizew;
		int le = sr - sl;
		if (le < 1) le = 0;
		else
			memcpy(pif->vdata[i]+dl, linef + sl, le);
		if (dl > 0)
		{
			memset(pif->vdata[i], *linef, dl);
		}
		else if (le < sy)
		{
			memset(pif->vdata[i] + le, linef[pif->sizew - 1], sy - le);
		}
	}
}

void macroblockPredGetDataUV(PBPredictInfo *pif, uint8_t *dat, int px, int py, int sx, int sy)
{
	for (int i = 0; i < sx; ++i)
	{
		int x = px + i;
		if (x<0) x = 0;
		else if (x >= pif->sizeh/2) x = pif->sizeh/2 - 1;
		uint8_t *linef = dat + x * pif->stride / 2;
		int dl = 0, sl = py, sr = py + sy;
		if (sl < 0)
		{
			dl -= sl;
			sl = 0;
		}
		if (sr > pif->sizew/2)
			sr = pif->sizew/2;
		int le = sr - sl;
		if (le < 0) le = 0;
		else if (le > 0)
			memcpy(pif->vdata[i]+dl, linef + sl, le);
		if (dl > 0)
		{
			memset(pif->vdata[i], *linef, dl);
		}
		else if (le < sy)
		{
			memset(pif->vdata[i] + le, linef[pif->sizew/2 - 1], sy - le);
		}
	}
}

// off_in_x, off_in_y 是4的倍数
void macroblockGetPred_axb(int listid, const refVector &ref, PBPredictInfo *pif, macroblockdata *pred, int sizex, int sizey, int off_in_x, int off_in_y)
{

	int px = pif->myr * 16 + off_in_x, py = pif->myc * 16 + off_in_y;
	int rx = px * 4 + ref.x, ry = py * 4 + ref.y;
	int subx = ref.x & 3, suby = ref.y & 3;
	int rtx = (rx - subx) / 4, rty = (ry - suby) / 4;
	int border = 2;
	//int i, j;

	if (subx==0 && suby==0) // Direct
	{  // Full-pel
		macroblockPredGetDataY(pif, pif->pdata_y0[listid][ref.refno], rtx, rty, sizex + 1, sizey + 1);
		for (int i = 0; i < sizex; ++i){
			int ioffinx = i+off_in_x;
			for (int j = 0; j < sizey; ++j)
				pred->y[ioffinx][j+off_in_y] = pif->vdata[i][j];
		}
	}
	else
	{
		int borderT = 0, borderL = 0;
		if (subx)
			borderT = 2;
		if (suby)
			borderL = 2;
		macroblockPredGetDataY(pif, pif->pdata_y0[listid][ref.refno], rtx - borderT, rty - borderL, borderT * 2 + sizex + 1, borderL * 2 + sizey + 1);
		macroblockGetHalfPel(pif, borderT * 2 + sizex + 1, borderL * 2 + sizey + 1);

		if ((subx&1)==0 && (suby&1)==0)
		{  // Half-pel
			int halfid = -1;
			if (subx)
				halfid += 2;
			if (suby)
				halfid += 1;
			for (int i = 0; i < sizex; ++i){
				int ioffinx = i+off_in_x;
				int iboarterT = i+borderT;
				for (int j = 0; j < sizey; ++j)
					pred->y[ioffinx][j+off_in_y] = pif->vdata_half[halfid][iboarterT][j+borderL];
			}
		}
		else
		{  // Quart-pel
			int boarderLplusone = borderL+1;
			int boarderTplusone = borderT+1;
			switch (subx)
			{
			case 0: 
				{
					switch(suby)
					{
					//case 0:
					case 1:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata[iboarterT][j+borderL] + pif->vdata_half[0][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					//case 2:
					case 3:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata[iboarterT][j+boarderLplusone] + pif->vdata_half[0][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					default:break;
					}
				}
				break;
			case 1: 
				{
					switch(suby)
					{
					case 0:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata[iboarterT][j+borderL] + pif->vdata_half[1][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 1:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][iboarterT][j+borderL] + pif->vdata_half[1][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 2:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][iboarterT][j+borderL] + pif->vdata_half[2][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 3:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][iboarterT][j+borderL] + pif->vdata_half[1][iboarterT][j+boarderLplusone] + 1) >> 1;
						}
						break;
					default:break;
					}
				}
				break;
			case 2: 
				{
					switch(suby)
					{
					//case 0:
					case 1:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[1][iboarterT][j+borderL] + pif->vdata_half[2][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					//case 2:
					case 3:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[1][iboarterT][j+boarderLplusone] + pif->vdata_half[2][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					default:break;
					}
				}
				break;
			case 3: 
				{
					switch(suby)
					{
					case 0:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata[i+boarderTplusone][j+borderL] + pif->vdata_half[1][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 1:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][i+boarderTplusone][j+borderL] + pif->vdata_half[1][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 2:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][i+boarderTplusone][j+borderL] + pif->vdata_half[2][iboarterT][j+borderL] + 1) >> 1;
						}
						break;
					case 3:
						for (int i = 0; i < sizex; ++i){
							int ioffinx = i+off_in_x;
							int iboarterT = i+borderT;
							for (int j = 0; j < sizey; ++j)
								pred->y[ioffinx][j+off_in_y] = (pif->vdata_half[0][i+boarderTplusone][j+borderL] + pif->vdata_half[1][iboarterT][j+boarderLplusone] + 1) >> 1;
						}
						break;
					default:break;
					}
				}
				break;
			default:break;
			}
		}
	}


	int off_in_x_half = off_in_x / 2, 
		off_in_y_half = off_in_y / 2,
		sizex_half = sizex / 2,
		sizey_half = sizey / 2;
	subx = ref.x & 7, suby = ref.y & 7;
	int C1 = (8-subx)*(8-suby), C2 = (8-subx)*(  suby), C3 = (  subx)*(8-suby), C4 = (  subx)*(  suby);
	int rutx = (rx - subx) / 8, ruty = (ry - suby) / 8;

	macroblockPredGetDataUV(pif, pif->pdata_u0[listid][ref.refno], rutx, ruty, sizex / 2 + 1, sizey / 2 + 1);
	if (subx==0 && suby==0)
	{
		for (int i = 0; i < sizex_half; ++i)
			for (int j = 0; j < sizey_half; ++j)
				pred->uv[0][i+off_in_x_half][j+off_in_y_half] = pif->vdata[i][j];
	}
	else
	{
		for (int i = 0; i < sizex_half; ++i)
			for (int j = 0; j < sizey_half; ++j)
			{
				pred->uv[0][i+off_in_x_half][j+off_in_y_half] = (
					C1*pif->vdata[i  ][j  ]+
					C2*pif->vdata[i  ][j+1]+
					C3*pif->vdata[i+1][j  ]+
					C4*pif->vdata[i+1][j+1]+
					32) >> 6;
			}
	}

	macroblockPredGetDataUV(pif, pif->pdata_v0[listid][ref.refno], rutx, ruty, sizex / 2 + 1, sizey / 2 + 1);
	if (subx==0 && suby==0)
	{
		for (int i = 0; i < sizex_half; ++i)
			for (int j = 0; j < sizey_half; ++j)
				pred->uv[1][i+off_in_x_half][j+off_in_y_half] = pif->vdata[i][j];
	}
	else
	{
		for (int i = 0; i < sizex_half; ++i)
			for (int j = 0; j < sizey_half; ++j)
			{
				pred->uv[1][i+off_in_x_half][j+off_in_y_half] = (
					C1*pif->vdata[i  ][j  ]+
					C2*pif->vdata[i  ][j+1]+
					C3*pif->vdata[i+1][j  ]+
					C4*pif->vdata[i+1][j+1]+
					32) >> 6;
			}
	}

}


// off_in_x, off_in_y 是4的倍数
void macroblockGetPred_axb_Bi(const refVector &ref0, const refVector &ref1, PBPredictInfo *pif, macroblockdata *pred, int sizex, int sizey, int off_in_x, int off_in_y)
{
	macroblockdata tempPred;
	macroblockGetPred_axb(0, ref0, pif, pred, sizex, sizey, off_in_x, off_in_y);
	macroblockGetPred_axb(1, ref1, pif, &tempPred, sizex, sizey, off_in_x, off_in_y);
	//int i, j;
	for (int i = 0; i < sizex; ++i){
		int ioffinx = i+off_in_x;
		for (int j = 0; j < sizey; ++j)
			pred->y[ioffinx][j+off_in_y] = (pred->y[ioffinx][j+off_in_y] + tempPred.y[ioffinx][j+off_in_y] + 1) >> 1;
	}
	int off_in_x_uv = off_in_x / 2, off_in_y_uv = off_in_y / 2;
	for (int i = 0; i < sizex/2; ++i){
		int ioffinuv = i+off_in_x_uv;
		for (int j = 0; j < sizey/2; ++j)
			pred->uv[0][ioffinuv][j+off_in_y_uv] = (pred->uv[0][ioffinuv][j+off_in_y_uv] + tempPred.uv[0][ioffinuv][j+off_in_y_uv] + 1) >> 1;
	}
	for (int i = 0; i < sizex/2; ++i){
		int ioffinuv = i+off_in_x_uv;
		for (int j = 0; j < sizey/2; ++j)
			pred->uv[1][ioffinuv][j+off_in_y_uv] = (pred->uv[1][ioffinuv][j+off_in_y_uv] + tempPred.uv[1][ioffinuv][j+off_in_y_uv] + 1) >> 1;
	}
}


// 计算一个Luma块的half-pel，使用Finite Impulse Response(FIR)
// Reference: H.264 and MPEG-4 Video Compression P173
void macroblockGetHalfPel(PBPredictInfo *pif, int hw, int hh)
{
	int tmp[24][24];
	for (int i = 0; i < hw; ++i)
		for (int j = 2; j < hh - 3; ++j)
		{
			pif->vdata_half[0][i][j] =
				CLIP1(
					  (pif->vdata[i][j-2]
				-  5 * pif->vdata[i][j-1]
				+ 20 * pif->vdata[i][j]
				+ 20 * pif->vdata[i][j+1]
				-  5 * pif->vdata[i][j+2]
				+      pif->vdata[i][j+3] + 16)
				/ 32);
			tmp[i][j] =
					   pif->vdata[i][j-2]
				-  5 * pif->vdata[i][j-1]
				+ 20 * pif->vdata[i][j]
				+ 20 * pif->vdata[i][j+1]
				-  5 * pif->vdata[i][j+2]
				+      pif->vdata[i][j+3];
		}
	
	for (int i = 2; i < hw - 3; ++i)
		for (int j = 0; j < hh; ++j)
		{
			pif->vdata_half[1][i][j] =
				CLIP1(
					  (pif->vdata[i-2][j]
				-  5 * pif->vdata[i-1][j]
				+ 20 * pif->vdata[i][j]
				+ 20 * pif->vdata[i+1][j]
				-  5 * pif->vdata[i+2][j]
				+      pif->vdata[i+3][j] + 16) 
				/ 32);
		}
	
	for (int i = 2; i < hw - 3; ++i)
		for (int j = 2; j < hh - 3; ++j)
		{
			pif->vdata_half[2][i][j] =
				CLIP1(
					  (tmp[i-2][j]
				-  5 * tmp[i-1][j]
				+ 20 * tmp[i][j]
				+ 20 * tmp[i+1][j]
				-  5 * tmp[i+2][j]
				+      tmp[i+3][j] + 512) 
				/ 1024);
		}
}

int getSAD(macroblockdata *data, macroblockdata *pred)
{
	int sad = 0;
	//int i, j;
	for (int i = 0; i < 16; ++i)
		for (int j = 0; j < 16; ++j)
			sad += labs((int)data->y[i][j] - (int)pred->y[i][j]);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			sad += labs((int)data->uv[0][i][j] - (int)pred->uv[0][i][j]);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
			sad += labs((int)data->uv[1][i][j] - (int)pred->uv[1][i][j]);
	return sad;
}

int getSAD_axb(macroblockdata *data, macroblockdata *pred, int sizex, int sizey, int off_in_x, int off_in_y)
{
	int sad = 0;
	//int i, j;
	for (int i = 0; i < sizex; ++i){
		int ioffinx = i+off_in_x;
		for (int j = 0; j < sizey; ++j)
			sad += labs((int)data->y[ioffinx][j+off_in_y] - (int)pred->y[ioffinx][j+off_in_y]);
	}
	int sizex_half = sizex / 2,
		sizey_half = sizey / 2,
		off_in_x_half = off_in_x / 2,
		off_in_y_half = off_in_y / 2;
	for (int i = 0; i < sizex_half; ++i)
		for (int j = 0; j < sizey_half; ++j)
			sad += labs((int)data->uv[0][i+off_in_x_half][j+off_in_y_half] - (int)pred->uv[0][i+off_in_x_half][j+off_in_y_half]);
	for (int i = 0; i < sizex_half; ++i)
		for (int j = 0; j < sizey_half; ++j)
			sad += labs((int)data->uv[1][i+off_in_x_half][j+off_in_y_half] - (int)pred->uv[1][i+off_in_x_half][j+off_in_y_half]);
	return sad;
}


int motionCost(int n_ref, int x, int y)
{
	// TODO: Calculate motion cost
	// Temporary usage
	int ret = 0;
	if (x<0)
	{
		++ret;
		x = -x;
	}
	if (y<0)
	{
		++ret;
		y = -y;
	}
	while (n_ref)
	{
		n_ref >>= 1;
		++ret;
	}
	while (x)
	{
		x >>= 1;
		++ret;
	}
	while (y)
	{
		y >>= 1;
		++ret;
	}
	return ret * 8;
}

//bool refsEqual(refVector *refs1, refVector *refs2)
//{
//	return (refs1[0].refno==refs2[0].refno &&
//		refs1[1].refno==refs2[1].refno &&
//		refs1[0].x==refs2[0].x &&
//		refs1[0].y==refs2[0].y &&
//		refs1[1].x==refs2[1].x &&
//		refs1[1].y==refs2[1].y
//		);
//}
//
//bool noRefs(refVector *refs)
//{
//	return (refs[0].refno < 0) && (refs[1].refno < 0);
//}

void macroblockPBPrediction(CodingPictInfo *cpi, PBPredictInfo *pif, macroblockdata *dif, macroblockInfo *mif, macroblockdata *pred)
{
	bool ok[4][4] = {0};
	//macroblockGetHalfPel(pif, cpi);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
		{
			int x = i, y = j;
			if (ok[i][j])
				continue;
			while ((x || y) && noRefs(mif->refs[x][y]))
			{
				printf("REF Error!!!\n");
				x = x & (x - 1);
				y = y & (y - 1);
			}
			int wd = 1, ht = 1;
			while (x+ht < 4 && (noRefs(mif->refs[x+ht][y]) || refsEqual(mif->refs[x+ht][y], mif->refs[x][y])))
			{
				++ht;
			}
			while (y+wd < 4)
			{
				bool sm = true;
				for (int k = 0; k < ht; ++k)
					if (!noRefs(mif->refs[x+k][y]) || !refsEqual(mif->refs[x+k][y],mif->refs[x+k][y+wd]))
					{
						sm = false;
						break;
					}
				if (!sm)
					break;
				++wd;
			}
			if ((mif->refs[x][y][0].refno < 0) && (mif->refs[x][y][1].refno < 0))
			{
				printf("REF Error!!!\n");
				refVector ref;
				ref.refno = 0;
				ref.x = 0;
				ref.y = 0;
				macroblockGetPred_axb(0, ref, pif, pred, 4*ht, 4*wd, i*4, j*4);
			}
			else if (mif->refs[x][y][0].refno < 0)
			{
				macroblockGetPred_axb(1, mif->refs[x][y][1], pif, pred, 4*ht, 4*wd, i*4, j*4);
			}
			else if (mif->refs[x][y][1].refno < 0)
			{
				macroblockGetPred_axb(0, mif->refs[x][y][0], pif, pred, 4*ht, 4*wd, i*4, j*4);
			}
			else
			{
				macroblockGetPred_axb_Bi(mif->refs[x][y][0], mif->refs[x][y][1], pif, pred, 4*ht, 4*wd, i*4, j*4);
			}
			for (int k = 0; k < ht; ++k)
				for (int l = 0; l < wd; ++l)
				{
					ok[x+k][y+l] = true;
				}
		}
}
