#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h" // GameplayTag
#include "SMWaveData.generated.h"


USTRUCT(BlueprintType)
struct FWaveSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawn")
    FDataTableRowHandle MonsterHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawn")
    int32 SpawnCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave Spawn")
    float SpawnInterval = 1.0f;
};

USTRUCT(BlueprintType)
struct FSMWaveData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    int32 WaveLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float MaintenanceTime = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TArray<FWaveSpawnData> SpawnList;
};
