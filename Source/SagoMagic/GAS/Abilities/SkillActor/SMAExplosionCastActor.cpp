#include "GAS/Abilities/SkillActor/SMAExplosionCastActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Building/SMBaseBuilding.h"
#include "Building/SMBaseCampActor.h"
#include "Character/SMPlayerCharacter.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"

ASMAExplosionCastActor::ASMAExplosionCastActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	PillarMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PillarMesh"));
	PillarMeshComponent->SetupAttachment(SceneRoot);
	PillarMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ChargeDiskMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChargeDiskMesh"));
	ChargeDiskMeshComponent->SetupAttachment(SceneRoot);
	ChargeDiskMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GroundMagicCircleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("GroundMagicCircle"));
	GroundMagicCircleComponent->SetupAttachment(SceneRoot);
	GroundMagicCircleComponent->bAutoActivate = false;

	ExplosionNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplosionNiagara"));
	ExplosionNiagaraComponent->SetupAttachment(SceneRoot);
	ExplosionNiagaraComponent->SetAutoActivate(false);

	ExplosionCascadeComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ExplosionCascade"));
	ExplosionCascadeComponent->SetupAttachment(SceneRoot);
	ExplosionCascadeComponent->bAutoActivate = false;
}

void ASMAExplosionCastActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMAExplosionCastActor, CastDuration);
	DOREPLIFETIME(ASMAExplosionCastActor, ExplosionRadius);
	DOREPLIFETIME(ASMAExplosionCastActor, VisualState);
}

void ASMAExplosionCastActor::BeginPlay()
{
	Super::BeginPlay();

	PillarMeshComponent->SetVisibility(false);
	ChargeDiskMeshComponent->SetVisibility(false);
	GroundMagicCircleComponent->Deactivate();
	ExplosionNiagaraComponent->Deactivate();
	ExplosionCascadeComponent->Deactivate();

	if (HasAuthority() == false)
	{
		OnRep_VisualState();
	}
}

void ASMAExplosionCastActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetNetMode() == NM_DedicatedServer || bIsCasting == false)
	{
		return;
	}

	ElapsedCastTime += DeltaSeconds;
	UpdateCastingVisuals();
}

void ASMAExplosionCastActor::InitializeCast(
	FGameplayEffectSpecHandle InDamageSpecHandle,
	AActor* InInstigatorActor,
	float InCastDuration,
	float InExplosionRadius)
{
	if (HasAuthority() == false)
	{
		return;
	}

	DamageSpecHandle = InDamageSpecHandle;
	InstigatorActor = InInstigatorActor;
	CastDuration = FMath::Max(InCastDuration, 0.01f);
	ExplosionRadius = FMath::Max(InExplosionRadius, 0.0f);
	ElapsedCastTime = 0.0f;
	bIsCasting = true;
	bHasExploded = false;
	VisualState = ESMExplosionCastVisualState::Casting;
	ForceNetUpdate();

	ApplyCastStartVisuals();
}

void ASMAExplosionCastActor::TriggerExplosion()
{
	if (HasAuthority() == false || bHasExploded)
	{
		return;
	}

	ApplyExplosionDamage();
	VisualState = ESMExplosionCastVisualState::Exploded;
	ForceNetUpdate();
	ApplyExplosionVisuals();
	SetLifeSpan(PostExplosionLifeSpan);
}

void ASMAExplosionCastActor::CancelCast()
{
	if (HasAuthority() == false || bHasExploded)
	{
		return;
	}

	Destroy();
}

void ASMAExplosionCastActor::OnRep_VisualState()
{
	switch (VisualState)
	{
	case ESMExplosionCastVisualState::Casting:
		ElapsedCastTime = 0.0f;
		ApplyCastStartVisuals();
		break;
	case ESMExplosionCastVisualState::Exploded:
		ApplyExplosionVisuals();
		break;
	case ESMExplosionCastVisualState::Cancelled:
		ApplyCancelVisuals();
		break;
	case ESMExplosionCastVisualState::Idle:
	default:
		break;
	}
}

void ASMAExplosionCastActor::ApplyCastStartVisuals()
{
	const float TargetScale = GetTargetVisualScale();
	const FVector PillarScale(PillarRadiusMultiplier, PillarRadiusMultiplier, PillarHeightScale);
	const FVector ExplosionScale(TargetScale, TargetScale, TargetScale);

	bIsCasting = true;
	bHasExploded = false;

	SetActorHiddenInGame(false);
	SetActorTickEnabled(GetNetMode() != NM_DedicatedServer);
	PillarMeshComponent->SetVisibility(true);
	PillarMeshComponent->SetRelativeScale3D(PillarScale);

	ChargeDiskMeshComponent->SetVisibility(true);
	ChargeDiskMeshComponent->SetRelativeScale3D(FVector::ZeroVector);

	GroundMagicCircleComponent->SetRelativeScale3D(FVector(TargetScale, TargetScale, 1.0f));
	GroundMagicCircleComponent->Activate(true);

	ExplosionNiagaraComponent->SetRelativeScale3D(ExplosionScale);
	ExplosionCascadeComponent->SetRelativeScale3D(ExplosionScale);
	ExplosionNiagaraComponent->Deactivate();
	ExplosionCascadeComponent->Deactivate();
}

void ASMAExplosionCastActor::ApplyExplosionVisuals()
{
	if (bHasExploded)
	{
		return;
	}

	bIsCasting = false;
	bHasExploded = true;

	SetActorHiddenInGame(false);
	SetActorTickEnabled(false);
	PillarMeshComponent->SetVisibility(false);
	ChargeDiskMeshComponent->SetVisibility(false);
	GroundMagicCircleComponent->Deactivate();
	ExplosionNiagaraComponent->SetRelativeScale3D(FVector(GetTargetVisualScale(), GetTargetVisualScale(), GetTargetVisualScale()));
	ExplosionCascadeComponent->SetRelativeScale3D(FVector(GetTargetVisualScale(), GetTargetVisualScale(), GetTargetVisualScale()));
	ExplosionNiagaraComponent->Activate(true);
	ExplosionCascadeComponent->Activate(true);
}

void ASMAExplosionCastActor::ApplyCancelVisuals()
{
	bIsCasting = false;
	bHasExploded = false;

	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	PillarMeshComponent->SetVisibility(false);
	ChargeDiskMeshComponent->SetVisibility(false);
	GroundMagicCircleComponent->Deactivate();
	ExplosionNiagaraComponent->Deactivate();
	ExplosionCascadeComponent->Deactivate();
}

void ASMAExplosionCastActor::UpdateCastingVisuals()
{
	const float Alpha = FMath::Clamp(ElapsedCastTime / CastDuration, 0.0f, 1.0f);
	const float TargetScale = GetTargetVisualScale();
	const float ChargeScale = Alpha * TargetScale;

	ChargeDiskMeshComponent->SetRelativeScale3D(FVector(ChargeScale, ChargeScale, 1.0f));
}

void ASMAExplosionCastActor::ApplyExplosionDamage()
{
	if (DamageSpecHandle.IsValid() == false)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = nullptr;
	if (InstigatorActor.IsValid())
	{
		SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor.Get());
	}

	if (SourceASC == nullptr)
	{
		return;
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	if (InstigatorActor.IsValid())
	{
		ActorsToIgnore.Add(InstigatorActor.Get());
	}

	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		ExplosionRadius,
		ObjectTypes,
		nullptr,
		ActorsToIgnore,
		OverlapActors);

	for (AActor* OverlapActor : OverlapActors)
	{
		if (IsValidExplosionTarget(OverlapActor) == false)
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlapActor);
		if (TargetASC == nullptr)
		{
			continue;
		}

		SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), TargetASC);
	}
}

float ASMAExplosionCastActor::GetTargetVisualScale() const
{
	const float SourceDiameter = FMath::Max(SourceMeshDiameterCm, 1.0f);
	const float TargetDiameter = FMath::Max(ExplosionRadius * 2.0f, 1.0f);
	return TargetDiameter / SourceDiameter;
}

bool ASMAExplosionCastActor::IsValidExplosionTarget(AActor* OtherActor) const
{
	if (OtherActor == nullptr)
	{
		return false;
	}

	if (OtherActor == this)
	{
		return false;
	}

	if (InstigatorActor.IsValid() && OtherActor == InstigatorActor.Get())
	{
		return false;
	}

	if (OtherActor->IsA<ASMPlayerCharacter>())
	{
		return false;
	}

	if (OtherActor->IsA<ASMBaseCampActor>() || OtherActor->IsA<ASMBaseBuilding>())
	{
		return false;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC == nullptr)
	{
		return false;
	}

	return TargetASC->HasMatchingGameplayTag(SMGameFlowTag::Team) == false;
}
