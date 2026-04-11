#pragma once

/** A* 경로탐색 노드 - 서버 전용 계산, 복제 불필요 */
struct FSMAStarNode
{
	FIntPoint Grid;
	FIntPoint Parent;
	FIntPoint Direction;
	int32 G = 0;
	int32 H = 0;
	int32 F = 0;
	
	bool operator>(const FSMAStarNode& Other) const
	{
		if (F == Other.F) return H > Other.H;
		return F > Other.F;
	}
};

/** 펜스 코너 판정 결과 - 로컬 계산용 */
struct FSMCornerInfo
{
	bool bIsCorner = false;
	float Yaw = 0.f;
};
