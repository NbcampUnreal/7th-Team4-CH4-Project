#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SMGameEnums.h"
#include "SMGemData.generated.h"


USTRUCT(BlueprintType)
struct FSMGemData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem|Info")
    FText GemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem|Tag")
    FGameplayTag GemTag;

    /** 이 젬을 장착할 수 있는 대상 스킬들의 태그 목록 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem|Tag")
    FGameplayTagContainer TargetTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem|Buff")
    EGemBuffType BuffType = EGemBuffType::None;

    /** 강화 수치 적용량 - ex. BuffType이 DamageMultiplier이고 값이 1.5 -> 최종 데미지에 1.5 곱함 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gem|Buff")
    float BuffValue = 1.0f;
};
