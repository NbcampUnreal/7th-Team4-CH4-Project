#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMASkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UGameplayEffect;

UCLASS()
class SAGOMAGIC_API ASMASkillProjectile : public AActor
{
    GENERATED_BODY()

public:
    ASMASkillProjectile();

    void InitProjectile(float InDamage, float InRangeCm, const FVector& InDirection, AActor* InInstigatorActor, AController* InController, TSubclassOf<UGameplayEffect> InDamageEffectClass);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    /** 비행 중 Flying 이펙트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UNiagaraComponent> FlyingEffect;

    /** 히트 시 이펙트 - BP에서 나이아가라 시스템 할당 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UNiagaraSystem> HitEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileSpeed = 1000.f;

private:

    float Damage = 0.f;
    float RangeCm = 0.f;
    FVector SpawnLocation = FVector::ZeroVector;
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    TWeakObjectPtr<AActor> InstigatorActor;
    TWeakObjectPtr<AController> InstigatorController;

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, FVector NormalImpulse,const FHitResult& Hit);

    //최대사거리 타이머 핸들러
    FTimerHandle TimerHandleMaxRange;
    void OnMaxRangeReached();
};
