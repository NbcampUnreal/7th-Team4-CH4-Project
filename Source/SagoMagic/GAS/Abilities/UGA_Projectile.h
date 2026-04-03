#pragma once

#include "UGA_SkillBase.h"
#include "SkillActor/SMASkillProjectile.h"
#include "UGA_Projectile.generated.h"

UCLASS()
class SAGOMAGIC_API UGA_Projectile : public UGA_SkillBase
{
    GENERATED_BODY()

public:
    UGA_Projectile();

protected:
    virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) override;

   
    UPROPERTY(EditDefaultsOnly, Category = "Skill")
    TSubclassOf<ASMASkillProjectile> ProjectileClass;
};
