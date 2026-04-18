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
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "SMASkillField.h"
#include "Building/SMBaseCampActor.h"
#include "Building/SMBaseBuilding.h"
#include "GAS/SMGameplayAbilityUtils.h"

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
                                         AActor* InInstigatorActor,
                                         bool bEnableHoming)
{
	DamageSpecHandle = InSpecHandle;
	RangeCm = InRangeCm;
	SpawnLocation = GetActorLocation();
	InstigatorActor = InInstigatorActor;

	//발사방향 -> 커서방향 값
	ProjectileMovement->Velocity = InDirection.GetSafeNormal() * ProjectileSpeed;

	if (bEnableHoming == true)
	{
		FindAndSetHomingTarget();
	}

	//최대 사거리 도달 시 없어지게
	if (RangeCm > 0.f && ProjectileSpeed > 0.f)
	{
		const float FlightTime = RangeCm / ProjectileSpeed;
		GetWorldTimerManager().SetTimer(TimerHandleMaxRange,
		                                this,
		                                &ASMASkillProjectile::OnMaxRangeReached,
		                                FlightTime,
		                                false);
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

	//같은 종류 프로젝타일은 무시
	if (Cast<ASMASkillProjectile>(OtherActor)) return;

	// 플레이어 캐릭터는 통과
	if (OtherActor->IsA<ASMPlayerCharacter>()) return;

	// 아군 오브젝트는 통과 장판, 베이스캠프, 건축물
	if (OtherActor->IsA<ASMASkillField>()) return;
	if (OtherActor->IsA<ASMBaseCampActor>()) return;
	if (OtherActor->IsA<ASMBaseBuilding>()) return;

	// 팀 태그 보유 액터 통과
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC && TargetASC->HasMatchingGameplayTag(SMGameFlowTag::Team)) return;

	// ASC 없는 액터 무시하고 통과
	if (!TargetASC || !DamageSpecHandle.IsValid()) return;

	UE_LOG(LogTemp, Warning, TEXT("[Projectile] Hit: %s"), *OtherActor->GetName());

	// 히트 GameplayCue 발동 - 실제 피격 대상에만 실행
	UAbilitySystemComponent* InstigatorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
		InstigatorActor.Get());
	if (InstigatorASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = SweepResult.ImpactPoint;
		CueParams.Normal = SweepResult.ImpactNormal;
		InstigatorASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_Projectile_Hit, CueParams);
	}

	// 직격 데미지
	TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

	// 폭발 이펙트 스폰
	if (ExplosionSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionSystem,
			GetActorLocation(),
			FRotator::ZeroRotator,
			FVector::OneVector,
			true,
			true,
			ENCPoolMethod::None);
	}

	// 2레벨 이상 스플래쉬 데미지
	if (SplashSpecHandle.IsValid() && SplashRadiusCm > 0.0f)
	{
		TArray<AActor*> SplashActors;

		UKismetSystemLibrary::SphereOverlapActors(
			this,
			GetActorLocation(),
			SplashRadiusCm,
			{UEngineTypes::ConvertToObjectType(ECC_Pawn)},
			APawn::StaticClass(),
			{this, OtherActor, InstigatorActor.Get()}, // 피격대상 중복 방지
			SplashActors);
		
		for (AActor* SplashTarget : SplashActors)
		{
			if (!IsValid(SplashTarget)) continue;
			
			UAbilitySystemComponent* SplashASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SplashTarget);
			
			if (!SplashASC) continue;
			if (SplashASC->HasMatchingGameplayTag(SMGameFlowTag::Team)) continue;
			
			SplashASC->ApplyGameplayEffectSpecToSelf(*SplashSpecHandle.Data.Get());
		}
	}

	Destroy();
}

void ASMASkillProjectile::OnMaxRangeReached()
{
	Destroy();
}

void ASMASkillProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASMASkillProjectile, HomingTarget);
}

void ASMASkillProjectile::OnRep_HomingTarget()
{
	if (!HomingTarget || !ProjectileMovement) return;

	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
	ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude;
}

void ASMASkillProjectile::FindAndSetHomingTarget()
{
	if (!ProjectileMovement) return;

	UE_LOG(LogTemp, Warning, TEXT("Start FindAndSetHomingTarget"));

	FVector CurrentDir = ProjectileMovement->Velocity.GetSafeNormal();

	//반경 내 액터만 수집
	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		HomingSearchRadius,
		{UEngineTypes::ConvertToObjectType(ECC_Pawn)},
		APawn::StaticClass(),
		{this},
		OverlappedActors);

	AActor* BestTarget = nullptr;
	float BestDot = -1.f;

	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor)) continue;
		if (!SMGameplayAbilityUtils::IsAvailableEnemy(Actor)) continue;
		FVector ToTarget = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(CurrentDir, ToTarget);
		if (Dot > BestDot)
		{
			BestDot = Dot;
			BestTarget = Actor;
		}
	}

	if (BestTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("BestTarget: %s"), *BestTarget->GetName());
		HomingTarget = BestTarget; //클라이언트 복제용
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = BestTarget->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = HomingAccelerationMagnitude;
	}
}

void ASMASkillProjectile::SetExplosionEffect(UNiagaraSystem* InExplosionSystem)
{
	ExplosionSystem = InExplosionSystem;
}

void ASMASkillProjectile::SetSplashConfig(FGameplayEffectSpecHandle InSplashSpecHandle, float InSplashRadiusCm)
{
	SplashSpecHandle = InSplashSpecHandle;
	SplashRadiusCm = InSplashRadiusCm;
}
