#include "Enemy/SMMonsterProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Enemy/SMMonsterBase.h"

ASMMonsterProjectile::ASMMonsterProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 충돌 콜리전
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(15.f);
	// QueryOnly: 물리 블로킹 없이 순수 오버랩 쿼리 → WorldStatic(펜스/건물) 포함 모든 오브젝트 감지
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore); // 다른 투사체 무시
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASMMonsterProjectile::OnProjectileOverlap);
	SetRootComponent(CollisionComponent);

	// 케스케이드 이펙트
	FlyingEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FlyingEffect"));
	FlyingEffect->SetupAttachment(CollisionComponent);
	FlyingEffect->SetAutoActivate(true);

	// 투사체 이동
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void ASMMonsterProjectile::InitProjectile(FGameplayEffectSpecHandle InSpecHandle,
                                           float InRangeCm,
                                           const FVector& InDirection,
                                           AActor* InInstigatorActor)
{
	DamageSpecHandle = InSpecHandle;
	RangeCm = InRangeCm;
	SpawnLocation = GetActorLocation();
	InstigatorActor = InInstigatorActor;

	// 발사 방향 설정
	ProjectileMovement->Velocity = InDirection.GetSafeNormal() * ProjectileSpeed;

	// 최대 사거리 도달 시 소멸 타이머
	if (RangeCm > 0.f && ProjectileSpeed > 0.f)
	{
		const float FlightTime = RangeCm / ProjectileSpeed;
		GetWorldTimerManager().SetTimer(
			TimerHandleMaxRange, this,
			&ASMMonsterProjectile::OnMaxRangeReached,
			FlightTime, false);
	}
}

void ASMMonsterProjectile::BeginPlay()
{
	Super::BeginPlay();
}

void ASMMonsterProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent,
                                                AActor* OtherActor,
                                                UPrimitiveComponent* OtherComponent,
                                                int32 OtherBodyIndex,
                                                bool bFromSweep,
                                                const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor) return;

	// InitProjectile 호출 전 Overlap 무시
	if (!InstigatorActor.IsValid()) return;

	// 자기 자신 또는 시전 몬스터 무시
	if (OtherActor == this) return;
	if (OtherActor == InstigatorActor.Get()) return;

	// 다른 몬스터는 무시
	if (OtherActor->IsA<ASMMonsterBase>()) return;

	// 대상 ASC에 데미지 적용
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC || !DamageSpecHandle.IsValid()) return;

	TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

	Destroy();
}

void ASMMonsterProjectile::OnMaxRangeReached()
{
	Destroy();
}
