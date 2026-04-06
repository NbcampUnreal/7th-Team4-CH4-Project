#include "SMASkillProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


ASMASkillProjectile::ASMASkillProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    //콜리전
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->InitSphereRadius(20.f);
    CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASMASkillProjectile::OnProjectileOverlap);
    SetRootComponent(CollisionComponent);

    //낭라가는 이펙트
    FlyingEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlyingEffect"));
    FlyingEffect->SetupAttachment(CollisionComponent);
    FlyingEffect->SetAutoActivate(true);

    //Projectile 이동
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ASMASkillProjectile::InitProjectile(float InDamage, float InRangeCm, const FVector& InDirection, AActor* InInstigatorActor, AController* InController, TSubclassOf<UGameplayEffect> InDamageEffectClass)
{
    //충돌 시 ApplySkillDamage에 전달 -> GE의 IncomingDamage -> AttributeSet(체력 차감)
    Damage = InDamage;
    RangeCm = InRangeCm;
    SpawnLocation = GetActorLocation();
    InstigatorActor = InInstigatorActor;
    InstigatorController = InController;
    DamageEffectClass = InDamageEffectClass;

    //발사방향 -> 커서방향 값
    ProjectileMovement->Velocity = InDirection.GetSafeNormal() * ProjectileSpeed;

    //최대 사거리 도달 시 없어지게
    if (RangeCm > 0.f && ProjectileSpeed > 0.f)
    {
        const float FlightTime = RangeCm / ProjectileSpeed;
        GetWorldTimerManager().SetTimer(TimerHandleMaxRange, this, &ASMASkillProjectile::OnMaxRangeReached, FlightTime, false);
    }
}


void ASMASkillProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void ASMASkillProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority()) return;
    if (!OtherActor) return;
    // InitProjectile 호출 전 스폰 직후 Overlap 무시
    if (!InstigatorActor.IsValid()) return;
    // 자기 자신 또는 instigator 무시
    if (OtherActor == this) return;
    if (OtherActor == InstigatorActor.Get()) return;

    UE_LOG(LogTemp, Warning, TEXT("[Projectile] Overlap: %s, IsPawn: %d, DamageEffectClass: %s"),
        *OtherActor->GetName(),
        OtherActor->IsA<APawn>(),
        DamageEffectClass ? *DamageEffectClass->GetName() : TEXT("NULL"));

    //히트 이펙트 재생
    if (HitEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitEffect, SweepResult.ImpactPoint, SweepResult.ImpactNormal.Rotation());
    }

    //액터에 닿으면 데미지 주고 사라지게
    if (OtherActor->IsA<APawn>())
    {
        USMAbilitySystemComponent::ApplySkillDamage(OtherActor, Damage, InstigatorActor.Get(), InstigatorController.Get(), DamageEffectClass);
    }
    Destroy();
}

void ASMASkillProjectile::OnMaxRangeReached()
{
    Destroy();
}


