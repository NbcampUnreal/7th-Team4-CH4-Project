#pragma once

#include "GA_SkillBase.h"
#include "SkillActor/SMASkillProjectile.h"
#include "GA_Projectile.generated.h"

UCLASS()
class SAGOMAGIC_API UGA_Projectile : public UGA_SkillBase
{
    GENERATED_BODY()

public:
    UGA_Projectile();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

   
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    TSubclassOf<ASMASkillProjectile> ProjectileClass;
};
