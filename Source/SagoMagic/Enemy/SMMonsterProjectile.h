#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMMonsterProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UParticleSystemComponent;

UCLASS()
class SAGOMAGIC_API ASMMonsterProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASMMonsterProjectile();

	/** GA에서 GE Spec, 사거리, 방향, 시전자를 전달받아 초기화 */
	void InitProjectile(FGameplayEffectSpecHandle InSpecHandle, float InRangeCm,
		const FVector& InDirection, AActor* InInstigatorActor);

protected:
	virtual void BeginPlay() override;

	/** 투사체 충돌 콜리전 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComponent;

	/** 투사체 이동 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/** 케스케이드 이펙트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UParticleSystemComponent> FlyingEffect;

	/** 투사체 속도 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
	float ProjectileSpeed = 500.f;

private:
	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	//최대 사거리 도달 시 소멸
	void OnMaxRangeReached();

	FGameplayEffectSpecHandle DamageSpecHandle;
	float RangeCm = 0.f;
	FVector SpawnLocation = FVector::ZeroVector;
	TWeakObjectPtr<AActor> InstigatorActor;
	FTimerHandle TimerHandleMaxRange;
};
