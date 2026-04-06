#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "SMSkillData.generated.h"


USTRUCT()
struct FSMSkillData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Info")
    FText SkillName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Tag")
    FGameplayTag SkillTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat")
    float BaseDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat")
    float Cooldown = 5.0f;

    /** 투사체 최대 사거리 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Combat")
    float RangeCm = 3000.0f;

    /** 어떤 시너지 젬도 장착하지 않았을 때 발사되는 기본 클래스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Asset")
    TSoftClassPtr<AActor> DefaultProjectile;

    /*시너지 젬 장착 시 변환될 발사체 클래스의 딕셔너리(맵)
     *TMap 구조를 통해 'Key: 특정 젬 태그'가 들어오면 'Value: 새로운 투사체'를 즉시 찾아냅니다.
     *검색 속도가 매우 빠르고, 추후 시너지 조합이 늘어나도 코드 수정 없이 데이터만 추가하면 됩니다.
     * >> GAS 부분은 이렇게 짜는 게 좋다고 하네요! 확인하셨으면 지우셔도 좋습니다~
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Synergy")
    TMap<FGameplayTag, TSoftClassPtr<AActor>> SynergyProjectiles;
};
