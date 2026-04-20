// GA_SkillTurret.h

#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "GA_SkillTurret.generated.h"

class ASMSkillTurret;

/**
 * 터렛 같은 걸 설치하고
 * 사정거리 내 가장 가까운 적을 자동으로 공격하는 스킬
 */
UCLASS()
class SAGOMAGIC_API UGA_SkillTurret : public UGA_SkillBase
{
	GENERATED_BODY()

protected:
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;
	
	/** 터렛 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<ASMSkillTurret> TurretClass;
	
	/** 스플래쉬용 GE - BP설정 필수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> SplashDamageEffectClass;
};
