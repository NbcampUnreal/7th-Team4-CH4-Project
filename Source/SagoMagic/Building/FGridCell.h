#pragma once

#include "CoreMinimal.h"
#include "FGridCell.generated.h"

UENUM(BlueprintType)
enum class EGridBuildingType : uint8
{
	None	UMETA(DisplayName = "None"),
	Fence	UMETA(DisplayName = "Fence"),
	Tower	UMETA(DisplayName = "Tower"),
};

UENUM(BlueprintType)
enum class ESelectState : uint8
{
	Idle,
	Selected,
	Moving
};

/**
 * 그리드 셀 하나의 상태
 * bIsOccupied / BuildingType / OwnerId -> GridData 배열과 함께 복제됨
 * PlacedActor -> 서버 전용 (건물 엑터는 bReplicates=true로 별도 복제)
 */
USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()
	
	/** 점유 여부 - 클라 프리뷰 유효성 체크에 사용 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsOccupied = false;
	
	/** 건물 타입 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EGridBuildingType BuildingType = EGridBuildingType::None;
	
	/**
	 * 배치된 액터 참조 - UPROPERTY 제거로 복제 페이로드에서 제외
	 * 건물 액터 자체가 bReplicates=true로 클라에 복제됨
	 * 서버 : GetActorAtCell() /ReplaceWithCorner() 등에서 사용
	 */
	 TWeakObjectPtr<AActor> PlacedActor = nullptr;
	
	/** 시공자 (플레이어 식별) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 OwnerId = -1;
	
	void Clear()
	{
		bIsOccupied = false;
		BuildingType = EGridBuildingType::None;
		PlacedActor = nullptr;
		OwnerId = -1;
	}
};
