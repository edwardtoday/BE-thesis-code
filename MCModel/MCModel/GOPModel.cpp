#include "GOPModel.h"

enum {PICT_IFRAME, PICT_PFRAME, PICT_BFRAME};

#include <set>
using namespace std;
extern int Debug_Insert;
//*/
//原始预测结构
const int GOPModel::frameTypes[8][8] = {
	{PICT_IFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
};

const double GOPModel::runningTime[8][8] = {
	{I_T, B1_T, B1_T, B2_T, B0_T, B1_T, B0_T, B1_T},
	{B0_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T},
	{P_T, B1_T, B1_T, B2_T, B0_T, B2_T, B1_T, B1_T},
	{B0_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T},
	{P_T, B1_T, B1_T, B2_T, B0_T, B2_T, B1_T, B1_T},
	{B0_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T, B2_T},
	{P_T, B1_T, B1_T, B2_T, B0_T, B2_T, B1_T, B1_T},
	{P_T, B1_T, B1_T, B2_T, B0_T, B2_T, B1_T, B1_T},
};

//const double GOPModel::runningTime[8][8] = {
//	{I_T, B9_T, B6_T, B12_T, B3_T, B12_T, B6_T, B9_T},
//	{B1_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
//	{P1_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
//	{B2_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
//	{P2_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
//	{B2_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
//	{P2_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
//	{P2_T, B16_T, B14_T, B17_T, B15_T, B17_T, B14_T, B16_T},
//};

const int GOPModel::effects[8][8][20][2] = {
// stream 0
{
{{1, 0}, {0, 1}, {0,-1}, {0, 2}, {0,-2}, {0, 4}, {0,-4}, {2, 0}, {-1,-1}},
{{1, 1},                                                 {-1,-1}},
{{1, 2}, {0, 1}, {0, 3},                                 {-1,-1}},
{{1, 3},                                                 {-1,-1}},
{{1, 4}, {0, 3}, {0, 5}, {0, 2}, {0, 6},                 {-1,-1}},
{{1, 5},                                                 {-1,-1}},
{{1, 6}, {0, 5}, {0, 7},                                 {-1,-1}},
{{1, 7},                                                 {-1,-1}}
},
// stream 1
{
{{1, 1}, {1,-1}, {1, 2}, {1,-2}, {1, 4}, {1,-4}, {-1,-1}},
{                                                {-1,-1}},
{{1, 1}, {1, 3},                                 {-1,-1}},
{                                                {-1,-1}},
{{1, 3}, {1, 5}, {1, 2}, {1, 6},                 {-1,-1}},
{                                                {-1,-1}},
{{1, 5}, {1, 7},                                 {-1,-1}},
{                                                {-1,-1}}
},
// stream 2
{
{{1, 0}, {3, 0}, {2, 1}, {2,-1}, {2, 2}, {2,-2}, {2, 4}, {2,-4}, {4, 0}, {-1,-1}},
{{1, 1}, {3, 1},                                                 {-1,-1}},
{{1, 2}, {3, 2}, {2, 1}, {2, 3},                                 {-1,-1}},
{{1, 3}, {3, 3},                                                 {-1,-1}},
{{1, 4}, {3, 4}, {2, 3}, {2, 5}, {2, 2}, {2, 6},                 {-1,-1}},
{{1, 5}, {3, 5},                                                 {-1,-1}},
{{1, 6}, {3, 6}, {2, 5}, {2, 7},                                 {-1,-1}},
{{1, 7}, {3, 7},                                                 {-1,-1}}
},
// stream 3
{
{{3, 1}, {3,-1}, {3, 2}, {3,-2}, {3, 4}, {3,-4}, {-1,-1}},
{                                                {-1,-1}},
{{3, 1}, {3, 3},                                 {-1,-1}},
{                                                {-1,-1}},
{{3, 3}, {3, 5}, {3, 2}, {3, 6},                 {-1,-1}},
{                                                {-1,-1}},
{{3, 5}, {3, 7},                                 {-1,-1}},
{                                                {-1,-1}}
},
// stream 4
{
{{3, 0}, {5, 0}, {4, 1}, {4,-1}, {4, 2}, {4,-2}, {4, 4}, {4,-4}, {6, 0}, {-1,-1}},
{{3, 1}, {5, 1},                                                 {-1,-1}},
{{3, 2}, {5, 2}, {4, 1}, {4, 3},                                 {-1,-1}},
{{3, 3}, {5, 3},                                                 {-1,-1}},
{{3, 4}, {5, 4}, {4, 3}, {4, 5}, {4, 2}, {4, 6},                 {-1,-1}},
{{3, 5}, {5, 5},                                                 {-1,-1}},
{{3, 6}, {5, 6}, {4, 5}, {4, 7},                                 {-1,-1}},
{{3, 7}, {5, 7},                                                 {-1,-1}}
},
// stream 5
{
{{5, 1}, {5,-1}, {5, 2}, {5,-2}, {5, 4}, {5,-4}, {-1,-1}},
{                                                {-1,-1}},
{{5, 1}, {5, 3},                                 {-1,-1}},
{                                                {-1,-1}},
{{5, 3}, {5, 5}, {5, 2}, {5, 6},                 {-1,-1}},
{                                                {-1,-1}},
{{5, 5}, {5, 7},                                 {-1,-1}},
{                                                {-1,-1}}
},
// stream 6
{
{{5, 0}, {7, 0}, {6, 1}, {6,-1}, {6, 2}, {6,-2}, {6, 4}, {6,-4}, {-1,-1}},
{{5, 1}, {7, 1},                                                 {-1,-1}},
{{5, 2}, {7, 2}, {6, 1}, {6, 3},                                 {-1,-1}},
{{5, 3}, {7, 3},                                                 {-1,-1}},
{{5, 4}, {7, 4}, {6, 3}, {6, 5}, {6, 2}, {6, 6},                 {-1,-1}},
{{5, 5}, {7, 5},                                                 {-1,-1}},
{{5, 6}, {7, 6}, {6, 5}, {6, 7},                                 {-1,-1}},
{{5, 7}, {7, 7},                                                 {-1,-1}}
},
// stream 7
{
{{7, 1}, {7,-1}, {7, 2}, {7,-2}, {7, 4}, {7,-4}, {-1,-1}},
{                                                {-1,-1}},
{{7, 1}, {7, 3},                                 {-1,-1}},
{                                                {-1,-1}},
{{7, 3}, {7, 5}, {7, 2}, {7, 6},                 {-1,-1}},
{                                                {-1,-1}},
{{7, 5}, {7, 7},                                 {-1,-1}},
{                                                {-1,-1}}
}
};
/*/
//改进的预测结构
const int GOPModel::frameTypes[8][8] = {
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_IFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
	{PICT_PFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME, PICT_BFRAME},
};

const double GOPModel::runningTime[8][8] = {
	{P2_T, B9_T, B6_T, B12_T, B3_T, B12_T, B6_T, B9_T},
	{B1_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
	{P2_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
	{B2_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
	{I_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
	{B2_T, B10_T, B8_T, B13_T, B5_T, B13_T, B8_T, B10_T},
	{P2_T, B11_T, B7_T, B12_T, B4_T, B12_T, B7_T, B11_T},
	{P2_T, B16_T, B14_T, B17_T, B15_T, B17_T, B14_T, B16_T},
};
const int GOPModel::effects[8][8][20][2] = {
	// stream 0
	{
		{{1, 0}, {0, 1}, {0,-1}, {0, 2}, {0,-2}, {0, 4}, {0,-4}, {-1,-1}},
		{{1, 1},                                                 {-1,-1}},
		{{1, 2}, {0, 1}, {0, 3},                                 {-1,-1}},
		{{1, 3},                                                 {-1,-1}},
		{{1, 4}, {0, 3}, {0, 5}, {0, 2}, {0, 6},                 {-1,-1}},
		{{1, 5},                                                 {-1,-1}},
		{{1, 6}, {0, 5}, {0, 7},                                 {-1,-1}},
		{{1, 7},                                                 {-1,-1}}
	},
		// stream 1
	{
		{{1, 1}, {1,-1}, {1, 2}, {1,-2}, {1, 4}, {1,-4}, {-1,-1}},
		{                                                {-1,-1}},
		{{1, 1}, {1, 3},                                 {-1,-1}},
		{                                                {-1,-1}},
		{{1, 3}, {1, 5}, {1, 2}, {1, 6},                 {-1,-1}},
		{                                                {-1,-1}},
		{{1, 5}, {1, 7},                                 {-1,-1}},
		{                                                {-1,-1}}
	},
		// stream 2
	{
		{{0, 0}, {1, 0}, {3, 0}, {2, 1}, {2,-1}, {2, 2}, {2,-2}, {2, 4}, {2,-4}, {-1,-1}},
		{{1, 1}, {3, 1},                                                 {-1,-1}},
		{{1, 2}, {3, 2}, {2, 1}, {2, 3},                                 {-1,-1}},
		{{1, 3}, {3, 3},                                                 {-1,-1}},
		{{1, 4}, {3, 4}, {2, 3}, {2, 5}, {2, 2}, {2, 6},                 {-1,-1}},
		{{1, 5}, {3, 5},                                                 {-1,-1}},
		{{1, 6}, {3, 6}, {2, 5}, {2, 7},                                 {-1,-1}},
		{{1, 7}, {3, 7},                                                 {-1,-1}}
	},
		// stream 3
	{
		{{3, 1}, {3,-1}, {3, 2}, {3,-2}, {3, 4}, {3,-4}, {-1,-1}},
		{                                                {-1,-1}},
		{{3, 1}, {3, 3},                                 {-1,-1}},
		{                                                {-1,-1}},
		{{3, 3}, {3, 5}, {3, 2}, {3, 6},                 {-1,-1}},
		{                                                {-1,-1}},
		{{3, 5}, {3, 7},                                 {-1,-1}},
		{                                                {-1,-1}}
	},
		// stream 4
	{
		{{2, 0}, {3, 0}, {5, 0}, {6, 0}, {4, 1}, {4,-1}, {4, 2}, {4,-2}, {4, 4}, {4,-4}, {-1,-1}},
		{{3, 1}, {5, 1},                                                 {-1,-1}},
		{{3, 2}, {5, 2}, {4, 1}, {4, 3},                                 {-1,-1}},
		{{3, 3}, {5, 3},                                                 {-1,-1}},
		{{3, 4}, {5, 4}, {4, 3}, {4, 5}, {4, 2}, {4, 6},                 {-1,-1}},
		{{3, 5}, {5, 5},                                                 {-1,-1}},
		{{3, 6}, {5, 6}, {4, 5}, {4, 7},                                 {-1,-1}},
		{{3, 7}, {5, 7},                                                 {-1,-1}}
	},
		// stream 5
	{
		{{5, 1}, {5,-1}, {5, 2}, {5,-2}, {5, 4}, {5,-4}, {-1,-1}},
		{                                                {-1,-1}},
		{{5, 1}, {5, 3},                                 {-1,-1}},
		{                                                {-1,-1}},
		{{5, 3}, {5, 5}, {5, 2}, {5, 6},                 {-1,-1}},
		{                                                {-1,-1}},
		{{5, 5}, {5, 7},                                 {-1,-1}},
		{                                                {-1,-1}}
	},
		// stream 6
	{
		{{5, 0}, {7, 0}, {6, 1}, {6,-1}, {6, 2}, {6,-2}, {6, 4}, {6,-4}, {-1,-1}},
		{{5, 1}, {7, 1},                                                 {-1,-1}},
		{{5, 2}, {7, 2}, {6, 1}, {6, 3},                                 {-1,-1}},
		{{5, 3}, {7, 3},                                                 {-1,-1}},
		{{5, 4}, {7, 4}, {6, 3}, {6, 5}, {6, 2}, {6, 6},                 {-1,-1}},
		{{5, 5}, {7, 5},                                                 {-1,-1}},
		{{5, 6}, {7, 6}, {6, 5}, {6, 7},                                 {-1,-1}},
		{{5, 7}, {7, 7},                                                 {-1,-1}}
	},
		// stream 7
	{
		{{7, 1}, {7,-1}, {7, 2}, {7,-2}, {7, 4}, {7,-4}, {-1,-1}},
		{                                                {-1,-1}},
		{{7, 1}, {7, 3},                                 {-1,-1}},
		{                                                {-1,-1}},
		{{7, 3}, {7, 5}, {7, 2}, {7, 6},                 {-1,-1}},
		{                                                {-1,-1}},
		{{7, 5}, {7, 7},                                 {-1,-1}},
		{                                                {-1,-1}}
	}
};
//*/


int GOPModel::depends[8][8][20][2];
int GOPModel::deps[8][8];
int GOPModel::effs[8][8];

bool GOPModel::GetNextProcessingPoc(int &frameId, int &streamId) // Poc
{
	bool ret = false;
	frameId = -1;
	streamId = -1;
	set<pair<int, int>>::iterator iter = canEncodings.begin();
	set<pair<int, int>>::iterator temp = canEncodings.begin();
	if (iter == canEncodings.end())
		ret = false;
	else
	{
		for (iter = canEncodings.begin(); iter != canEncodings.end(); iter ++)
		{
			int GOPnum = iter->first / GOP_length;
			if (GOPnum - current_GOP < GOP_limit)  // with in GOP limit
			{
				if (iter->first <= temp->first)  // To find the smallest frameId
				{
					temp = iter;
					ret = true;
				}
			}
		}
		if (ret)
		{
			frameId = temp->first;
			streamId = temp->second;
			canEncodings.erase(temp);
			ret = true;
		}
	}
	return ret;
}

bool GOPModel::GetNextProcessingRef(int &frameId, int &streamId) // Ref
{
	bool ret = false;
	frameId = -1;
	streamId = -1;
	set<pair<int, int>>::iterator iter = canEncodings.begin();
	set<pair<int, int>>::iterator temp = canEncodings.begin();
	if (iter == canEncodings.end())
		ret = false;
	else
	{
		for (iter = canEncodings.begin(); iter != canEncodings.end(); iter ++)
		{
			int GOPnum = iter->first / GOP_length;
			if (GOPnum - current_GOP < GOP_limit)  // with in GOP limit
			{
				if (effs[iter->first & 7][iter->second] >= effs[temp->first & 7][temp->second])  // To find the largest effs
				{
					temp = iter;
					ret = true;
				}
			}
		}

		if (ret)
		{
			frameId = temp->first;
			streamId = temp->second;
			canEncodings.erase(temp);
			ret = true;
		}
	}
	return ret;
}

bool GOPModel::GetNextProcessingExt(int &frameId, int &streamId) // Ext
{
	bool ret = false;
	frameId = -1;
	streamId = -1;
	set<pair<int, int>>::iterator iter = canEncodings.begin();
	set<pair<int, int>>::iterator temp = canEncodings.begin();
	if (iter == canEncodings.end())
		ret = false;
	else
	{
		for (iter = canEncodings.begin(); iter != canEncodings.end(); iter ++)
		{
			int GOPnum = iter->first / GOP_length;
			if (GOPnum - current_GOP < GOP_limit)  // with in GOP limit
			{
				if (runningTime[iter->first & 7][iter->second] <= runningTime[temp->first & 7][temp->second])  // To find the smallest frameId
				{
					temp = iter;
					ret = true;
				}
			}
		}

		if (ret)
		{
			frameId = temp->first;
			streamId = temp->second;
			canEncodings.erase(temp);
			ret = true;
		}
	}
	return ret;
}

double GOPModel::calcExeTime(int frameId, int streamId)
{
	if (frameId <0 || streamId <0) return 0;
	frameId = frameId & 7;
	double temp = 0;
	for (int k = 0;; k++)
	{
		
		const int *eft = GOPModel::effects[frameId][streamId][k];
		if (eft[0] < 0) 
			break;
		double next = calcExeTime(eft[0], eft[1]);
		if (next >= temp)
		{
			temp = next;
		}
	}
	
	temp += runningTime[streamId][frameId];  //by ppsnow
	//temp += runningTime[frameId][streamId];
	//printf("执行时间:%f\n",temp);
	return temp;
}

double GOPModel::calcLftFrame(int frameId, int streamId)
{
	if (frameId <0 || streamId <0) return 0;
	frameId = frameId & 7;
	double temp = 0;
	for (int k = 0;; k++)
	{
		
		const int *eft = GOPModel::effects[frameId][streamId][k];
		if (eft[0] < 0) break;
		double next = calcExeTime(eft[0], eft[1]);
		if (next >= temp)
		{
			temp = next;
		}
	}
	temp += 1;
	return temp;
}

bool GOPModel::GetNextProcessingSum(int &frameId, int &streamId) // Sum
{
	bool ret = false;
	frameId = -1;
	streamId = -1;
	set<pair<int, int>>::iterator iter = canEncodings.begin();
	set<pair<int, int>>::iterator temp = canEncodings.begin();
	if (iter == canEncodings.end())
		ret = false;
	else
	{
		for (iter = canEncodings.begin(); iter != canEncodings.end(); iter ++)
		{
			int GOPnum = iter->first / GOP_length;
			if (GOPnum - current_GOP < GOP_limit)  // with in GOP limit
			{
				if (calcExeTime(iter->first, iter->second) >= calcExeTime(temp->first, temp->second))  // To find the max sum of the forward path.
				{
					temp = iter;
					ret = true;
				}
			}
		}
		if (ret)
		{
			frameId = temp->first;
			streamId = temp->second;
			//printf("(%d,%d)执行时间:%f\n",frameId,streamId,calcExeTime(temp->first,temp->second));
			canEncodings.erase(temp);
			ret = true;
		}
	}
	return ret;
}

bool GOPModel::GetNextProcessingLft(int &frameId, int &streamId) // Lft
{
	bool ret = false;
	frameId = -1;
	streamId = -1;
	set<pair<int, int>>::iterator iter = canEncodings.begin();
	set<pair<int, int>>::iterator temp = canEncodings.begin();
	if (iter == canEncodings.end())
		ret = false;
	else
	{
		for (iter = canEncodings.begin(); iter != canEncodings.end(); iter ++)
		{
			int GOPnum = iter->first / GOP_length;
			if (GOPnum - current_GOP < GOP_limit)  // with in GOP limit
			{
				if (calcLftFrame(iter->first, iter->second) >= calcLftFrame(temp->first, temp->second))  // To find the smallest frameId
				{
					temp = iter;
					ret = true;
				}
			}
		}

		if (ret)
		{
			frameId = temp->first;
			streamId = temp->second;
			canEncodings.erase(temp);
			ret = true;
		}
	}
	return ret;
}

bool GOPModel::ProcessCompleted()
{
	return (this->n_Encoded  >= GOPModel::frameCount * 8);
}
void GOPModel::Initialize(int limit)
{
	GOP_limit = limit;
	current_GOP = 0;
	sliceCount = new int[GOP_limit];
	memset(sliceCount, 0, GOP_limit * sizeof(int));
//	this->GOPModel::frameCount = GOPModel::frameCount;
	this->n_Encoded = 0;
	memset(deps, 0, sizeof(deps[0][0]) * 8 * 8);
	memset(effs, 0, sizeof(effs[0][0]) * 8 * 8);
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
		{
			for (int k = 0; ; ++k)
			{
				const int *eft = effects[i][j][k];		
				if (eft[0] < 0)	//k的最大值，直接在退出时赋值
				{		
					effs[i][j] = k;
					break;
				}
				if (eft[1] < 0)//一般都是每个view的第一帧
				{
					int tdeps = deps[eft[0]][eft[1]+8]; //依赖帧的数目
					depends[eft[0]][eft[1]+8][tdeps][0] = i;
					depends[eft[0]][eft[1]+8][tdeps][1] = j+8; //实际上即为0，因为一般此时j=0
					++deps[eft[0]][eft[1]+8];
				}
				else
				{
					int tdeps = deps[eft[0]][eft[1]];
					depends[eft[0]][eft[1]][tdeps][0] = i;
					depends[eft[0]][eft[1]][tdeps][1] = j;
					++deps[eft[0]][eft[1]];
				}
			}
		}

	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j)
		{
			depends[i][j][deps[i][j]][0] = -1;
			depends[i][j][deps[i][j]][1] = -1;
		}

	for (int i = 0; i < 8; ++i)
	{
		dep_frames[i] = new int[GOPModel::frameCount];
		eff_frames[i] = new int[GOPModel::frameCount];
	}
	canEncodings.clear();
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < GOPModel::frameCount; ++j)
		{
			int baseTimeId = j & ~7; //GOP的起始位置
			dep_frames[i][j] = 0;
			for (int k = 0; k < deps[i][j&7]; ++k)
			{
				int timeId = depends[i][j&7][k][1] + baseTimeId;
				if (timeId >= 0 && timeId < GOPModel::frameCount)
					++dep_frames[i][j];
			}
			if (dep_frames[i][j] == 0)	//没有依赖关系的，即为I帧，插入
				canEncodings.insert(make_pair(j, i));

			eff_frames[i][j] = 1;
			for (int k = 0; k < effs[i][j&7]; ++k)
			{
				int timeId = effects[i][j&7][k][1] + baseTimeId;
				if (timeId >= 0 && timeId < GOPModel::frameCount)
					++eff_frames[i][j];
			}
		}
}

void GOPModel::OnFrameComplete(int frameId, int streamId)
{
	int baseTimeId = frameId & ~7;
	for (int k = 0; ; ++k)
	{
		const int *eft = effects[streamId][frameId&7][k];
		if (eft[0] < 0)
			break;
		int timeId = eft[1]+baseTimeId;
		if (timeId >= 0 && timeId < GOPModel::frameCount)
		{
			if (!--dep_frames[eft[0]][timeId])
			{
				canEncodings.insert(make_pair(eft[1]+baseTimeId, eft[0]));
				if (Debug_Insert == 1)
				{
					printf("Insert an frame: viewId, FrameId:\t<%d,%d>\n",eft[0],eft[1]+baseTimeId);
				}
			}
		}
	}

	this->n_Encoded ++;
	this->sliceCount[frameId / GOP_length % GOP_limit]++;
	if (sliceCount[current_GOP % GOP_limit] == GOP_length * STREAM_COUNT)
	{
		sliceCount[current_GOP % GOP_limit] = 0;
		current_GOP ++;
	}

	//printf(" (%d, %d), encoded = %d, frame_count * 8 = %d, ", frameId, streamId, this->n_Encoded, GOPModel::frameCount * 8);//---debug

	for (int k = 0; ; ++k)
	{
		int *dep = depends[streamId][frameId&7][k];
		if (dep[0] < 0)
			break;
		--eff_frames[streamId][frameId]; // !!!

		int timeId = dep[1]+baseTimeId;  // ???
		if (timeId >= 0 && timeId < GOPModel::frameCount)
		{
			--eff_frames[dep[0]][timeId];
		}
	}
}

void GOPModel::update(int FrameId, int StreamId)
{
	if (FrameId + StreamId >= 0)
		OnFrameComplete(FrameId, StreamId);
}

double GOPModel::GetFrameTime(int FrameId, int StreamId)
{
	if (FrameId + StreamId >= 0)
		return GOPModel::runningTime[StreamId][FrameId & 7];
	else
		return -1;
}