#pragma once

#include "CoreMinimal.h"
#include "SMLogData.generated.h"


USTRUCT()
struct FSMLogData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor TextColor = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 2.0f;
};
