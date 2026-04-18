#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "GA_Explosion.generated.h"

class ASMAExplosionCastActor;

/**
 * Explosion 어빌리티 정의 파일
 *
 * 포함 내용:
 * - 캐스팅 시작 및 종료 처리
 * - 타겟 위치 해석
 * - 캐스터 이동 고정 처리
 * - 캐스팅 액터 및 타이머 상태
 *
 * 역할:
 * - 지점 지정 후 일정 시간 캐스팅을 유지하면 폭발을 발생시키는 스킬의 흐름 정의
 */

/** Explosion 어빌리티 클래스 */
UCLASS()
class SAGOMAGIC_API UGA_Explosion : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	UGA_Explosion();

	/** 어빌리티 활성화 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	/** 어빌리티 종료 */
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** 스킬 효과 실행 */
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;

protected:
	/** 캐스팅 중 스폰할 연출 액터 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Explosion")
	TSubclassOf<ASMAExplosionCastActor> ExplosionCastActorClass;

	/** Explosion의 고정 캐스팅 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Casting")
	float FixedCastTime = 1.5f;

	/** 클릭 가능한 최대 사정거리. Range/Area 수치와 무관하게 고정입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Targeting")
	float TargetingRangeCm = 1000.0f;

private:
	/** 캐스팅 완료 시점 처리 */
	UFUNCTION()
	void HandleCastFinished();

	/** 공격 입력 릴리즈 이벤트 처리 */
	UFUNCTION()
	void OnAttackReleasedEventReceived(FGameplayEventData Payload);

	/** 서버 측 캐스팅 시작 */
	void BeginServerCast(const FGameplayAbilityActorInfo* ActorInfo, const FVector& TargetLocation);

	/** 캐스팅 완료 타이머 시작 */
	void StartCastTimer(float InCastTime);

	/** 서버로 전달된 타겟 데이터 수신 */
	void OnExplosionTargetDataReady(
		const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		FGameplayTag ApplicationTag);

	/** 타겟 위치 계산 */
	bool ResolveTargetLocation(const FGameplayAbilityActorInfo* ActorInfo, FVector& OutTargetLocation) const;

	/** 캐스터 이동 고정 여부 적용 */
	void SetCasterMovementLocked(const FGameplayAbilityActorInfo* ActorInfo, bool bLocked) const;

	/** 실제 캐스팅 시간 반환 */
	float GetResolvedCastTime() const;

	/** Skill Summary 기준 실제 폭발 반경 반환 */
	float GetResolvedExplosionRadius() const;

private:
	/** 캐스팅 완료 대기 타이머 핸들 */
	FTimerHandle CastTimerHandle;

	/** 계산된 목표 지점 */
	FVector ResolvedTargetLocation = FVector::ZeroVector;

	/** 캐스팅 완료 처리 진입 여부 */
	bool bCastFinished = false;

	/** 서버 측 캐스팅 시작 여부 */
	bool bServerCastStarted = false;

	/** 현재 활성화된 캐스팅 연출 액터 */
	UPROPERTY()
	TObjectPtr<ASMAExplosionCastActor> ActiveCastActor;
};
