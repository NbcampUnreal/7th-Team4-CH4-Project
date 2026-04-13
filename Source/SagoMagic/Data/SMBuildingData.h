#pragma once

#include "CoreMinimal.h"
#include "Building/FGridCell.h"
#include "Engine/DataTable.h"
#include "SMBuildingData.generated.h"

class UTexture2D;
class USoundBase;
class AActor;

USTRUCT(BlueprintType)
struct FSMBuildingData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText DisplayName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UTexture2D> Icon = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EGridBuildingType BuildingType = EGridBuildingType::None;
    
    /** 골드 비용 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Cost = 10;
    
    /** 차지하는 셀 크기 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FIntPoint GridSize = FIntPoint(1, 1);
    
    /** 스폰할 액터 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AActor> BuildingClass = nullptr;
    
    /** 몬스터 경로를 막는지 여부 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bBlocksPath = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsDestructible = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USoundBase> PlaceSound = nullptr;
    
};
