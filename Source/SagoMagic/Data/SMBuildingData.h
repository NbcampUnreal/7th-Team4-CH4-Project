#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SMBuildingData.generated.h"

USTRUCT(BlueprintType)
struct FSMBuildingData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Info")
    FText BuildingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Cost")
    int32 CostGold = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Stat")
    float MaxHP = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building|Asset")
    TSoftObjectPtr<UStaticMesh> BuildingMesh;
};
