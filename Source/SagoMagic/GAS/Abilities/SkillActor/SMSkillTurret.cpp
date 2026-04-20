// SMSkillTurret.cpp


#include "SMSkillTurret.h"

#include "NiagaraComponent.h"
#include "SMASkillProjectile.h"
#include "GAS/SMGameplayAbilityUtils.h"
#include "Kismet/KismetSystemLibrary.h"


ASMSkillTurret::ASMSkillTurret()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	TurretEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TurretEffect"));
	SetRootComponent(TurretEffect);
	TurretEffect->SetAutoActivate(true);
}

void ASMSkillTurret::InitTurret(
	FGameplayEffectSpecHandle InSpecHandle,
	FGameplayEffectSpecHandle InSplashSpecHandle,
	AActor* InInstigatorActor,
	float InDuration,
	float InRangeCm,
	float InFireInterval,
	int32 InSkillLevel)
{
	DamageSpecHandle = InSpecHandle;
	SplashSpecHandle = InSplashSpecHandle;
	InstigatorActor = InInstigatorActor;
	SkillLevel = InSkillLevel;
	
	if (InRangeCm > 0.0f) RangeCm = InRangeCm;
	
	const float Interval = InFireInterval > 0.0f ? InFireInterval : DefaultFireInterval;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	// 터렛 지속시간 타이머
	World->GetTimerManager().SetTimer(
		DurationEndHandle, this, &ThisClass::OnDurationExpired, InDuration, false);
	
	// 터렛 발사 간격 타이머
	World->GetTimerManager().SetTimer(
		FireTimerHandle, this, &ThisClass::FireAtNearestEnemy, Interval, true, Interval);
}

void ASMSkillTurret::FireAtNearestEnemy()
{
	if (!HasAuthority() || !ProjectileClass || !DamageSpecHandle.IsValid()) return;
	
	// 범위 안에 들어온 액터
	TArray<AActor*> OverlappedActors;
	
	// 검색 대상에 터렛 자신은
	TArray<AActor*> ActorsToIgnore = {this};
	
	// 검색에 설치한 플레이어는 무시
	if (InstigatorActor.IsValid())
	{
		ActorsToIgnore.Add(InstigatorActor.Get());
	}
	
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		RangeCm,
		{UEngineTypes::ConvertToObjectType(ECC_Pawn)},
		APawn::StaticClass(),
		ActorsToIgnore,
		OverlappedActors);
	
	AActor* NearestEnemy = nullptr;
	float NearestDist = FLT_MAX;
	
	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor)) continue;
		if (!SMGameplayAbilityUtils::IsAvailableEnemy(Actor)) continue;
		
		const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestEnemy = Actor;
		}
	}
	
	if (!IsValid(NearestEnemy)) return;
	
	const FVector BaseDirection =
		(NearestEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	
	SpawnProjectile(BaseDirection);
	
	
	if (SkillLevel >= 3)
	{
		// 딜레이 후 두 번째 발사
		CachedFireDirection = BaseDirection;
		GetWorldTimerManager().SetTimer(
			SecondShotTimerHandle,
			this,
			&ThisClass::FireSecondShot,
			DualFireDelay,
			false);
	}
}

void ASMSkillTurret::SpawnProjectile(const FVector& Direction)
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	FActorSpawnParameters SpawnPrams;
	SpawnPrams.Owner = InstigatorActor.IsValid() ? InstigatorActor.Get() : this;
	SpawnPrams.Instigator = InstigatorActor.IsValid() ? Cast<APawn>(InstigatorActor.Get()) : nullptr;
	SpawnPrams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ASMASkillProjectile* Projectile = World->SpawnActor<ASMASkillProjectile>(
		ProjectileClass, GetActorLocation(), Direction.Rotation(), SpawnPrams);
	
	if (!IsValid(Projectile)) return;
	
	Projectile->InitProjectile(DamageSpecHandle, RangeCm, Direction, InstigatorActor.Get(), false);
	
	if (SkillLevel >= 2 && SplashSpecHandle.IsValid())
	{
		Projectile->SetSplashConfig(SplashSpecHandle, SplashRadiusCm);
	}
}

void ASMSkillTurret::OnDurationExpired()
{
	Destroy();
}

void ASMSkillTurret::FireSecondShot()
{
	SpawnProjectile(CachedFireDirection);
}
