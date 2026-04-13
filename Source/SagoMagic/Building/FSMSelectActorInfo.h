#pragma once

#include "CoreMinimal.h"
#include "FGridCell.h"
#include "FSMSelectActorInfo.generated.h"

class AActor;
class UMaterialInterface;

/** 편집 모드 선택 액터 정보 - 클라이언트 전용 UI 상태 */
USTRUCT()
struct FSMSelectActorInfo
{
	GENERATED_BODY()
	
	UPROPERTY()
	TWeakObjectPtr<AActor> Actor = nullptr;
	
	FIntPoint OriginalGridPos = FIntPoint(0, 0);
	float OriginalYaw = 0.f;
	EGridBuildingType BuildingType = EGridBuildingType::None;
	int32 OriginalOwnerId = -1;
	
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
	
	FVector PivotOffset = FVector::ZeroVector;
};
