#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMASkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class SAGOMAGIC_API ASMASkillProjectile : public AActor
{
    GENERATED_BODY()

public:
    ASMASkillProjectile();

    // GA에서 미리 만든 GE Spec을 받아 보관, 충돌 시 TargetASC에 직접 적용
    void InitProjectile(FGameplayEffectSpecHandle InSpecHandle, float InRangeCm,
        const FVector& InDirection, AActor* InInstigatorActor);

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
    
    UFUNCTION()
    void OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
    
    void OnMaxRangeReached();
    
    // Spec핸들로 관리
    FGameplayEffectSpecHandle DamageSpecHandle;
    float RangeCm = 0.f;
    FVector SpawnLocation = FVector::ZeroVector;

    TWeakObjectPtr<AActor> InstigatorActor;

    FTimerHandle TimerHandleMaxRange;
};
