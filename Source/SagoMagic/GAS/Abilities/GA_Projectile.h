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
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<ASMASkillProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float SpreadAngle = 15.f;
	
	int32 ProjectileCount = 1;
	
	bool bEnableHoming = false;
};
