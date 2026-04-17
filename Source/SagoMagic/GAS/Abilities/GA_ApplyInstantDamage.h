// GA_ApplyInstantDamage.h

#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "GA_ApplyInstantDamage.generated.h"

class USoundBase;

/**
 * 마우스 커서 위치 기준 원형 범위 내 가장 가까운 적에게 즉시 낙뢰 데미지를 주는 스킬입니다.
 * 범위 내 적이 없으면 어빌리티가 취소되며 쿨다운이 소비되지 않습니다.
 */
UCLASS()
class SAGOMAGIC_API UGA_ApplyInstantDamage : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_ApplyInstantDamage();

protected:
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;

	/** 커서 위치 기준 적 탐색 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DetectionRadius")
	float DetectionRadius = 300.f;

	/** 시전 사운드 - 항상 재생  */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> CastSound;

	/** 어택 사운드 - 적 발견 시 추가 재생 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Debug")
	bool bShowDebugSphere = true;

private:
	//지정 위치 기준 반경 내 가장 가까운 적을 찾음 성공 시 true, OutEnemy에 결과 저장
	bool FindClosestEnemy(
		UWorld* World,
		const FVector& Center,
		float Radius,
		const AActor* IgnoreActor,
		AActor*& OutEnemy) const;

	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetDataHandle, FGameplayTag ApplicationTag);
};
