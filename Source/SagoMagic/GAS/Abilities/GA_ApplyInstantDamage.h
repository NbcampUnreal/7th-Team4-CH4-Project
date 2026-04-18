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

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	virtual void OnMontageFinished() override;

	//일반 공격
	void NormalAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center);

	//단일 타겟에 데미지 + Cue 적용
	void ApplyDamageAndCue(const FGameplayAbilityActorInfo* ActorInfo, APawn* Avatar, AActor* Target);

	/** 커서 위치 기준 적 탐색 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DetectionRadius")
	float DetectionRadius = 300.f;

	/** 시전 사운드 - 항상 재생  */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> CastSound;
	
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Debug")
	bool bShowDebugSphere = true;

private:
	// 공격할 적 숫자
	int32 TargetCount = 1;

	//지정 위치 기준 반경 내 가까운 적들을 MaxCount만큼 찾음. 성공 시 true, OutEnemy에 결과 저장
	bool FindClosestEnemies(
		UWorld* World,
		const FVector& Center,
		float Radius,
		const AActor* IgnoreActor,
		int32 MaxCount,
		TArray<AActor*>& OutEnemies) const;

	//다중 적 공격
	void InstantMultiAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center);

	bool bIsInstantMulti = false;

	/** InstantMulti 업그레이드 적용시 동시에 공격할 적 숫자 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Upgrade")
	int32 InstantMultiTargetCount = 3;

	//다중 적 다중 공격
	void SeparateMultiAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center);

	void FireNextLightningBolt();

	bool bIsSeparateMulti = false;

	/** SeparateMulti 업그레이드 적용시 동시에 공격할 적 숫자 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Upgrade")
	int32 SeparateMultiTargetCount = 4;
	/** SeparateMulti 업그레이드 적용시 떨어질 공격 숫자 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Upgrade")
	int32 SeparateMultiLightningCount = 7;
	/** SeparateMulti 업그레이드 적용시 떨어질 공격 딜레이 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Upgrade")
	float LightningDelay = 0.1f;

	UPROPERTY()
	TArray<AActor*> EnemyCandidates;
	UPROPERTY()
	TWeakObjectPtr<AActor> LastHitTarget;

	int32 RemainingLightnings = 0;
	bool bLightningPending = false;
	FTimerHandle LightningTimerHandle;
};
