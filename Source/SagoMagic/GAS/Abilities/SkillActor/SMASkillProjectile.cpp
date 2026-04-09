#include "SMASkillProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/SMPlayerCharacter.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"

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

	//비행 이펙트
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

void ASMASkillProjectile::InitProjectile(FGameplayEffectSpecHandle InSpecHandle,
                                         float InRangeCm,
                                         const FVector& InDirection,
                                         AActor* InInstigatorActor)
{
	DamageSpecHandle = InSpecHandle;
	RangeCm = InRangeCm;
	SpawnLocation = GetActorLocation();
	InstigatorActor = InInstigatorActor;

	//발사방향 -> 커서방향 값
	ProjectileMovement->Velocity = InDirection.GetSafeNormal() * ProjectileSpeed;

	//최대 사거리 도달 시 없어지게
	if (RangeCm > 0.f && ProjectileSpeed > 0.f)
	{
		const float FlightTime = RangeCm / ProjectileSpeed;
		GetWorldTimerManager().SetTimer(TimerHandleMaxRange, this,
		                                &ASMASkillProjectile::OnMaxRangeReached, FlightTime, false);
	}
}

void ASMASkillProjectile::BeginPlay()
{
    Super::BeginPlay();
}

void ASMASkillProjectile::OnProjectileOverlap(UPrimitiveComponent* OverlappedComponent,
                                              AActor* OtherActor,
                                              UPrimitiveComponent* OtherComponent,
                                              int32 OtherBodyIndex,
                                              bool bFromSweep,
                                              const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor) return;
	// InitProjectile 호출 전 스폰 직후 Overlap 무시
	if (!InstigatorActor.IsValid()) return;
	// 자기 자신 또는 다른 시전자 무시
	if (OtherActor == this) return;
	if (OtherActor == InstigatorActor.Get()) return;

	// 플레이어 캐릭터는 통과
	if (OtherActor->IsA<ASMPlayerCharacter>()) return;

    // 팀 태그 보유 액터 통과
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    if (TargetASC && TargetASC->HasMatchingGameplayTag(SMGameFlowTag::Team)) return;

	UE_LOG(LogTemp, Warning, TEXT("[Projectile] Hit: %s"), *OtherActor->GetName());

	// 히트 GameplayCue 발동 GCN_ProjectileHit에서 이펙트 처리
	UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
		InstigatorActor.Get());
	if (InstigatorASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SweepResult.ImpactPoint;
		CueParams.Normal = SweepResult.ImpactNormal;
		InstigatorASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_Projectile_Hit, CueParams);
	}

	// GA에서 미리 만든 Spec을 TargetASC에 직접 적용
	if (TargetASC && DamageSpecHandle.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	}

	Destroy();
}

void ASMASkillProjectile::OnMaxRangeReached()
{
	Destroy();
}
