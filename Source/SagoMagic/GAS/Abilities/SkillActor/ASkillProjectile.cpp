#include "ASkillProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/SMAbilitySystemComponent.h"


AASkillProjectile::AASkillProjectile()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    //콜리전
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    CollisionComponent->InitSphereRadius(20.f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->OnComponentHit.AddDynamic(this, &AASkillProjectile::OnProjectileHit);
    SetRootComponent(CollisionComponent);

    //메쉬
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    MeshComponent->SetupAttachment(CollisionComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //Projectile 이동
    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->UpdatedComponent = CollisionComponent;
    ProjectileMovement->InitialSpeed = ProjectileSpeed;
    ProjectileMovement->MaxSpeed = ProjectileSpeed;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AASkillProjectile::InitProjectile(float InDamage, float InRangeCm, const FVector& InDirection,
    AActor* InInstigatorActor, AController* InController)
{
    //충돌 시 ApplySkillDamage에 전달 -> GE의 IncomingDamage -> AttributeSet(체력 차감)
    Damage = InDamage;
    RangeCm = InRangeCm;
    SpawnLocation = GetActorLocation();
    InstigatorActor = InInstigatorActor;
    InstigatorController = InController;

    //발사방향 -> 커서방향 값
    ProjectileMovement->Velocity = InDirection.GetSafeNormal() * ProjectileSpeed;

    //최대 사거리 도달 시 없어지게
    if (RangeCm > 0.f && ProjectileSpeed > 0.f)
    {
        const float FlightTime = RangeCm / ProjectileSpeed;
        GetWorldTimerManager().SetTimer(TimerHandleMaxRange, this, &AASkillProjectile::OnMaxRangeReached, FlightTime, false);
    }
}


void AASkillProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void AASkillProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!HasAuthority()) return;
    if (!OtherActor || OtherActor == InstigatorActor.Get()) return;

    //액터에 닿으면 ApplyDamage로 데미지 주고 사라지게
    if (OtherActor->IsA<APawn>())
    {
        //TODO: SMMonsterBase::GetAbilitySystemComponent() 가 AbilitySystemComponent를반환해야 GAS 경로가 동작함
        //USMAbilitySystemComponent::ApplyDamage(OtherActor, Damage, InstigatorActor.Get(), InstigatorController.Get());
    }
    Destroy();
}

void AASkillProjectile::OnMaxRangeReached()
{
    Destroy();
}


