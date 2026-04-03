#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMASkillProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class SAGOMAGIC_API ASMASkillProjectile : public AActor
{
    GENERATED_BODY()

public:
    ASMASkillProjectile();

    void InitProjectile(float InDamage, float InRangeCm, const FVector& InDirection, AActor* InInstigatorActor, AController* InController);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float ProjectileSpeed = 1000.f;

private:

    float Damage = 0.f;
    float RangeCm = 0.f;
    FVector SpawnLocation = FVector::ZeroVector;

    TWeakObjectPtr<AActor> InstigatorActor;
    TWeakObjectPtr<AController> InstigatorController;

    UFUNCTION()
    void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,UPrimitiveComponent* OtherComponent, FVector NormalImpulse,const FHitResult& Hit);

    //최대사거리 타이머 핸들러
    FTimerHandle TimerHandleMaxRange;
    void OnMaxRangeReached();
};
