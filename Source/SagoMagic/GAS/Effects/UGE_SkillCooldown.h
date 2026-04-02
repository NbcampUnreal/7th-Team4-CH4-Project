#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "UGE_SkillCooldown.generated.h"

/**
 * HasDuration
 *
 * Duration을 SetByCaller(Data.Cooldown)으로 받아 n초 동안 유지
 * SKillBase::ApplyCooldown에서 CooldownSeconds 값을 넣어서 적용
 *
 * 각 GA의 ActivationBlockedTags에 해당하는 Cooldown.Skill.스킬타입 태그를 등록하면
 * 이 이펙트가 살아있는 동안 스킬 재상용이 자동으로 차단된다
 */
UCLASS()
class SAGOMAGIC_API UGE_SkillCooldown : public UGameplayEffect
{
    GENERATED_BODY()
    UGE_SkillCooldown();
};
