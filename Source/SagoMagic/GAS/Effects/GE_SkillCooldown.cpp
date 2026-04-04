#include "GE_SkillCooldown.h"
#include "GameplayTags/Character/SMSkillTag.h"

UGE_SkillCooldown::UGE_SkillCooldown()
{
    DurationPolicy = EGameplayEffectDurationType::HasDuration;

    // Duration 은 UGA_SkillBase::ApplyCooldown() 에서 SetByCaller로 주입.
    FSetByCallerFloat SetByCallerDuration;
    SetByCallerDuration.DataTag = SMSkillTag::Data_Cooldown;
    //지속시간 수치 대입, Caller에서 정해주는 값에 따라 결정.
    DurationMagnitude = FGameplayEffectModifierMagnitude(SetByCallerDuration);

    //쿨다운이 걸려 있을 때 중첩되면 최대 중첩 횟수 1로 고정, 다시 걸리면 시간 초기화.
    StackingType               = EGameplayEffectStackingType::AggregateByTarget;
    StackLimitCount            = 1;
    StackDurationRefreshPolicy = EGameplayEffectStackingDurationPolicy::RefreshOnSuccessfulApplication;
}
