#pragma once

#include "UGA_SkillBase.h"
#include "UGA_Projectile.generated.h"

/**
 *
 */
UCLASS()
class SAGOMAGIC_API UGA_Projectile : public UGA_SkillBase
{
    GENERATED_BODY()

public:
    UGA_Projectile();

protected:
    virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) override;

    //TODO : 풀리퀘 하고 나서 주석 풀기
    //UPROPERTY(EditDefaultsOnly, Category = "Skill")
    //TSubclassOf<SMASkillProjectile> ProjectileClass;
};
