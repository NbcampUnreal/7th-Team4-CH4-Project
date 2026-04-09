#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMASkillField.generated.h"

class UBoxComponent;
class UAbilitySystemComponent;

UCLASS()
class SAGOMAGIC_API ASMASkillField : public AActor
{
	GENERATED_BODY()

public:
	ASMASkillField();

	void InitField(FGameplayEffectSpecHandle InSpecHandle, AActor* InInstigatorActor,
		float InDuration);

protected:
	virtual void BeginPlay() override;

	/** 장판 범위 콜리전 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Field")
	TObjectPtr<UBoxComponent> CollisionComponent;

	/** 장판 박스 범위 - BP에서 조정 가능 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Field")
	FVector FieldBoxExtent = FVector(500.f, 500.f, 100.f);

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

	//지속시간
	float Duration = 5.f;
};
