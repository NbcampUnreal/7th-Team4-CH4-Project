#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_SkillDamage.generated.h"

/**
 * 스킬 피해 InstantDamage 상위 클래스
 * 직접 사용하지않고 InstantDamage를 통해 사용
 */
UCLASS()
class SAGOMAGIC_API UGE_SkillDamage : public UGameplayEffect
{
    GENERATED_BODY()

public:
    UGE_SkillDamage();
};
