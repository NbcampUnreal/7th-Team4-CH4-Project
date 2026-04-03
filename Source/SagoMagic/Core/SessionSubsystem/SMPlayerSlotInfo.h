//SMPlayerSlotInfo.h

#pragma once

#include "CoreMinimal.h"
#include "SMPlayerSlotInfo.generated.h"

/**
 * UI 표시용 플레이어 슬롯 정보
 */
USTRUCT(BlueprintType)
struct FSMPlayerSlotInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString PlayerName = TEXT("");

    UPROPERTY(BlueprintReadOnly)
    bool bIsReady = false;

    UPROPERTY(BlueprintReadOnly)
    bool bIsHost = false;
};

