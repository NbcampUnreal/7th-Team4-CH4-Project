#include "GAS/Abilities/UGA_SkillBase.h"
#include "AbilitySystemComponent.h"

UGA_SkillBase::UGA_SkillBase()
{
    //** 어빌리티 인스턴스 생성 - 단 하나 */
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    //** 어빌리티가 네트워크 어디서 실행될지 - 클라이언트 */
    NetExecutionPolicy =EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    //** 인스턴스를 네트워크로 복제할지 - 복제안함 */
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
}

bool UGA_SkillBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_SkillBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}


FString UGA_SkillBase::GetCurrentPredictionKeyStatus()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool UGA_SkillBase::IsPredictionKeyValidForMorePrediction() const
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

