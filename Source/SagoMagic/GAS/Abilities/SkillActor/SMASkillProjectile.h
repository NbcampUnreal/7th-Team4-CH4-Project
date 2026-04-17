#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "SMASkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;

UCLASS()
class SAGOMAGIC_API ASMASkillProjectile : public AActor
{
    GENERATED_BODY()

public:
    ASMASkillProjectile();

    // GA에서 미리 만든 GE Spec을 받아 보관, 충돌 시 TargetASC에 직접 적용
    void InitProjectile(FGameplayEffectSpecHandle InSpecHandle, float InRangeCm,
        const FVector& InDirection, AActor* InInstigatorActor, bool bEnableHoming = false);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    /** 비행 중 비주얼 이펙트 - BP에서 나이아가라 에셋 할당 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
    TObjectPtr<UNiagaraComponent> FlyingEffect;

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
    
    // 호밍기능
private:
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    
    UPROPERTY(ReplicatedUsing= OnRep_HomingTarget)
    TObjectPtr<AActor> HomingTarget;
    
    UFUNCTION()
    void OnRep_HomingTarget();
protected:
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float HomingAccelerationMagnitude = 3000.f;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float HomingSearchRadius = 1500.f;
    
private:
    
    void FindAndSetHomingTarget();
    
    bool IsAvailableEnemy(AActor* Actor) const;
};
