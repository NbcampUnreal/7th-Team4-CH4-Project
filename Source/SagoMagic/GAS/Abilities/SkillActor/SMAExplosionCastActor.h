#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMAExplosionCastActor.generated.h"

class UNiagaraComponent;
class UAudioComponent;
class UParticleSystemComponent;
class USceneComponent;
class UStaticMeshComponent;

UENUM()
enum class ESMExplosionCastVisualState : uint8
{
	Idle,
	Casting,
	Exploded,
	Cancelled
};

/**
 * Explosion 캐스팅 연출 액터 정의 파일
 *
 * 포함 내용:
 * - 캐스팅 초기화 및 진행 갱신
 * - 폭발 발동 및 취소 처리
 * - 캐스팅/폭발 연출 컴포넌트
 * - 데미지 적용에 필요한 상태 값
 *
 * 역할:
 * - Explosion 스킬의 캐스팅 중 연출과 발동 시 연출을 하나의 액터에서 관리
 */

/** Explosion 캐스팅 연출 액터 */
UCLASS()
class SAGOMAGIC_API ASMAExplosionCastActor : public AActor
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	ASMAExplosionCastActor();

	/** 캐스팅 경과 시간에 따라 연출 갱신 */
	virtual void Tick(float DeltaSeconds) override;

	/** 복제 대상 등록 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** 캐스팅에 필요한 데이터 초기화 */
	void InitializeCast(
		FGameplayEffectSpecHandle InDamageSpecHandle,
		AActor* InInstigatorActor,
		float InCastDuration,
		float InExplosionRadius);

	/** 폭발 연출 및 데미지 적용 */
	void TriggerExplosion();

	/** 캐스팅 연출 취소 */
	void CancelCast();

protected:
	/** 액터 시작 시 초기 연출 상태 설정 */
	virtual void BeginPlay() override;

	/** 액터 종료 시 루프 사운드 정리 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	/** 루트 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 하늘로 뻗는 기둥 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Casting")
	TObjectPtr<UStaticMeshComponent> PillarMeshComponent;

	/** 캐스팅 진행도를 표현하는 확장 디스크 메시 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Casting")
	TObjectPtr<UStaticMeshComponent> ChargeDiskMeshComponent;

	/** 기본 바닥 마법진 Cascade 파티클 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Casting")
	TObjectPtr<UParticleSystemComponent> GroundMagicCircleComponent;

	/** 실제 발동 시 재생할 Niagara 폭발 이펙트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Explosion")
	TObjectPtr<UNiagaraComponent> ExplosionNiagaraComponent;

	/** 실제 발동 시 재생할 Cascade 폭발 이펙트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explosion|Explosion")
	TObjectPtr<UParticleSystemComponent> ExplosionCascadeComponent;

	/** 원판/마법진 메시의 기본 지름 cm. 기본 Plane 100cm 기준 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Visual")
	float SourceMeshDiameterCm = 100.0f;

	/** 기둥 메시 XY 배율 보정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Visual")
	float PillarRadiusMultiplier = 1.0f;

	/** 기둥 메시 Z 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Visual")
	float PillarHeightScale = 10.0f;

	/** 충전 디스크 최소 스케일. 완전 0 대신 약간 남겨서 렌더링 튐을 줄입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Visual")
	float MinChargeVisualScale = 0.02f;

	/** 폭발 후 액터 유지 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Visual")
	float PostExplosionLifeSpan = 2.0f;

	/** 실제 폭발 판정 반경에 곱할 보정 배율 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Explosion|Gameplay")
	float ExplosionDamageRadiusMultiplier = 1.5f;

private:
	/** 복제된 시각 상태 반영 */
	UFUNCTION()
	void OnRep_VisualState();

	/** 캐스팅 시작 비주얼 적용 */
	void ApplyCastStartVisuals();

	/** 폭발 비주얼 적용 */
	void ApplyExplosionVisuals();

	/** 취소 비주얼 적용 */
	void ApplyCancelVisuals();

	/** 현재 캐스팅 경과에 맞춰 연출 상태 갱신 */
	void UpdateCastingVisuals();

	/** 폭발 범위 내 대상에게 데미지 적용 */
	void ApplyExplosionDamage();

	/** 폭발 반경 기반 목표 비주얼 스케일 계산 */
	float GetTargetVisualScale() const;

	/** 폭발 적용 대상 유효성 검사 */
	bool IsValidExplosionTarget(AActor* OtherActor) const;

	/** 차지 루프 사운드 시작 */
	void PlayCastingLoopSound();

	/** 차지 루프 사운드 정지 */
	void StopCastingLoopSound();

private:
	/** 실제 적용할 데미지 스펙 핸들 */
	FGameplayEffectSpecHandle DamageSpecHandle;

	/** 데미지 유발자 */
	TWeakObjectPtr<AActor> InstigatorActor;

	/** 전체 캐스팅 시간 */
	UPROPERTY(Replicated)
	float CastDuration = 1.0f;

	/** 실제 폭발 반경 */
	UPROPERTY(Replicated)
	float ExplosionRadius = 300.0f;

	/** 현재 누적 캐스팅 시간 */
	float ElapsedCastTime = 0.0f;

	/** 차지 중 재생되는 루프 사운드 컴포넌트 */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CastingLoopAudioComponent = nullptr;

	/** 기둥 메시 기본 스케일 */
	FVector PillarBaseScale = FVector::OneVector;

	/** 충전 디스크 기본 스케일 */
	FVector ChargeDiskBaseScale = FVector::OneVector;

	/** 바닥 마법진 기본 스케일 */
	FVector GroundMagicCircleBaseScale = FVector::OneVector;

	/** 폭발 나이아가라 기본 스케일 */
	FVector ExplosionNiagaraBaseScale = FVector::OneVector;

	/** 폭발 캐스케이드 기본 스케일 */
	FVector ExplosionCascadeBaseScale = FVector::OneVector;

	/** 캐스팅 진행 여부 */
	bool bIsCasting = false;

	/** 폭발 발동 여부 */
	bool bHasExploded = false;

	/** 현재 복제된 시각 상태 */
	UPROPERTY(ReplicatedUsing = OnRep_VisualState)
	ESMExplosionCastVisualState VisualState = ESMExplosionCastVisualState::Idle;
};
