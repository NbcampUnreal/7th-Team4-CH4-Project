#pragma once
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Data/SMBuildingData.h"
#include "SMBuildPlaceTargetData.generated.h"

/** 각 셀의 배치 정보 */
USTRUCT()
struct FSMCellPlaceInfo
{
	GENERATED_BODY()
	
	FIntPoint Grid = FIntPoint::ZeroValue;
	float Yaw = 0.f;
	bool bIsCorner = false;
};

/** GA로 전달할 배치 요청 데이터 */
USTRUCT()
struct FSMBuildPlaceTargetData : public FGameplayAbilityTargetData
{
	GENERATED_BODY()
	
	EGridBuildingType BuildingType = EGridBuildingType::None;
	TArray<FSMCellPlaceInfo> CellInfos;
	
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
	{
		
		Ar << BuildingType;
		
		int32 Count = CellInfos.Num();
		Ar << Count;
		CellInfos.SetNum(Count);
		
		for (int32 i = 0; i < Count; ++i)
		{
			Ar << CellInfos[i].Grid.X;
			Ar << CellInfos[i].Grid.Y;
			Ar << CellInfos[i].Yaw;
			Ar << CellInfos[i].bIsCorner;
		}
		
		bOutSuccess = true;
		return true;
	}
	
	TSharedPtr<FGameplayAbilityTargetData> Duplicate() const 
	{
		return MakeShared<FSMBuildPlaceTargetData>(*this);
	}
};

template<>
struct TStructOpsTypeTraits<FSMBuildPlaceTargetData> 
	: public TStructOpsTypeTraitsBase2<FSMBuildPlaceTargetData>
{
	enum { WithNetSerializer = true };
};
