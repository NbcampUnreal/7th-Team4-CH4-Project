#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMASkillField.generated.h"

class USphereComponent;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class SAGOMAGIC_API ASMASkillField : public AActor
{
	GENERATED_BODY()

public:
	ASMASkillField();

	void InitField(FGameplayEffectSpecHandle InSpecHandle, AActor* InInstigatorActor,
		float InDuration, float InRangeCm, bool bEnablePull = false, bool bEnableSlow = false);
	
public:
	float GetFieldDuration() const { return Duration; }
	float GetFieldRangeCm() const { return FieldRadius; }

protected:
	virtual void BeginPlay() override;

	/** 장판 범위 콜리전 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Field")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** 장판 구체 반경 - BP에서 조정 가능 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field")
	float FieldRadius = 500.f;

	/** 스폰 직후 즉시 데미지 방지용 딜레이 - BP에서 조정 가능 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field")
	float StartDelay = 0.5f;
	
	/** 당기는 힘 cm/s */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field|Pull")
	float PullStrength = 500.f;

	/** 당기기 적용 주기 초 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field|Pull")
	float PullInterval = 0.1f;
	
	/** 이동속도 감소 배율 0.5 = 50%로 감소 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field|Slow")
	float SlowMultiplier = 0.5f;

private:
	UFUNCTION()
	void OnFieldBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnFieldEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	FGameplayEffectSpecHandle DamageSpecHandle;

	// Duration 경과 후 모든 GE 제거 및 액터 소멸
	void OnDurationExpired();

	TWeakObjectPtr<AActor> InstigatorActor;

	// 범위 내 액터별 적용된 GE 핸들 저장 EndOverlap 시 제거용
	TMap<AActor*, FActiveGameplayEffectHandle> ActiveEffectHandles;

	FTimerHandle DurationEndHandle;

	// 스폰 직후 즉시 데미지 방지용 딜레이 타이머
	FTimerHandle InitialOverlapHandle;

	// 딜레이 후 스폰 시점 오버랩 액터 처리
	void CheckInitialOverlaps();
	
	UPROPERTY(EditDefaultsOnly, Category = "FieldEffects")
	TSubclassOf<UGameplayEffect> CueEffectClass;
	
	FActiveGameplayEffectHandle CueEffectHandle;
	
	//지속시간
	float Duration = 5.f;
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
	FGameplayTag ActiveCueTag;
	
	FTimerHandle PullTimerHandle;

	bool bSlowEnabled = false;

	// 범위 내 액터 원래 이동속도 저장 슬로우 해제용
	TMap<AActor*, float> OriginalMoveSpeeds;

	void ApplyPull();
	void ApplySlow(AActor* Actor);
	void RestoreSpeed(AActor* Actor);
};
