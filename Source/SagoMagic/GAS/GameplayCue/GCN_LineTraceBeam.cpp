// GCN_LineTraceBeam.cpp


#include "GAS/GameplayCue/GCN_LineTraceBeam.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"


AGCN_LineTraceBeam::AGCN_LineTraceBeam()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // OnActive에서 수동 활성화

	bAutoDestroyOnRemove = true; //OnRemove 후 자동 삭제

	BeamNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BeamNiagaraComponent"));
	BeamNiagaraComponent->SetupAttachment(RootComponent);
	BeamNiagaraComponent->SetAutoActivate(false);
}

bool AGCN_LineTraceBeam::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	InitializeBeam(MyTarget, Parameters);
	return true;
}

bool AGCN_LineTraceBeam::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::WhileActive_Implementation(MyTarget, Parameters);
	InitializeBeam(MyTarget, Parameters);
	return true;
}

bool AGCN_LineTraceBeam::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	SetActorTickEnabled(false);

	if (IsValid(BeamNiagaraComponent) == true)
	{
		BeamNiagaraComponent->Deactivate();
	}

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGCN_LineTraceBeam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBeam();
}

void AGCN_LineTraceBeam::InitializeBeam(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (IsValid(BeamNiagaraComponent) && BeamNiagaraComponent->IsActive())
	{
		return;
	}

	TargetActor = MyTarget;
	BeamRange = Parameters.RawMagnitude; //GA_LineTrace에서 RangeCm으로 전달한 값
	bPenetrate = Parameters.NormalizedMagnitude >= 1.f;

	if (IsValid(BeamNiagaraSystem) == false) return;
	BeamNiagaraComponent->SetAsset(BeamNiagaraSystem);
	
	ACharacter* Character = Cast<ACharacter>(MyTarget);
	if (IsValid(Character) == false || IsValid(Character->GetMesh()) == false) return;
	BeamNiagaraComponent->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	BeamNiagaraComponent->Activate(true);
	SetActorTickEnabled(true);
}

void AGCN_LineTraceBeam::UpdateBeam()
{
	if (IsValid(BeamNiagaraComponent) == false || TargetActor.IsValid() == false) return;

	ACharacter* Character = Cast<ACharacter>(TargetActor.Get());
	if (IsValid(Character) == false) return;

	//Staff_Tip소켓 위치를 LineTrace시작점으로 사용
	FVector Origin = GetAttachSocketLocation(Character);
	BeamNiagaraComponent->SetWorldLocation(Origin);
	
	FVector AimDirection = Character->GetBaseAimRotation().Vector();

	AController* Controller = Character->GetController();
	if (IsValid(Controller) == true)
	{
		//스킬 사용플레이어인 경우 Controller 방향으로
		AimDirection = Controller->GetControlRotation().Vector();
	}

	const FVector TraceEnd = Origin + AimDirection * BeamRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TargetActor.Get());

	//LineTraceMulti로 아군 관통 처리
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, Origin, TraceEnd, ECC_Pawn, Params);

	FVector BeamEndPoint = TraceEnd;
	if (bPenetrate == false)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (IsValid(HitActor) == false) continue;

			if (HasAnyTeamTag(HitActor) == true) continue;

			BeamEndPoint = Hit.ImpactPoint;
			break;
		}
	}
	//Niagara USER파라미터 "BeamEnd" 갱신
	BeamNiagaraComponent->SetVariableVec3(TEXT("BeamEnd"), BeamEndPoint);
}

bool AGCN_LineTraceBeam::HasAnyTeamTag(AActor* Actor) const
{
	if (IsValid(Actor) == false) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (IsValid(ASC) == false) return false;

	return ASC->HasMatchingGameplayTag(SMGameFlowTag::Team);
}

FVector AGCN_LineTraceBeam::GetAttachSocketLocation(ACharacter* Character) const
{
	// WeaponSocket에 붙은 StaticMesh에서 탐색
	TArray<UStaticMeshComponent*> Comps;
	Character->GetComponents<UStaticMeshComponent>(Comps);
	for (UStaticMeshComponent* Comp : Comps)
	{
		if (IsValid(Comp) == false) continue;
		if (Comp->GetAttachSocketName() != FName("WeaponSocket")) continue;
		if (Comp->DoesSocketExist(AttachSocketName))
		{
			return Comp->GetSocketLocation(AttachSocketName);
		}
		break;
	}

	// 못 찾으면 캐릭터 루트 위치 반환
	return Character->GetActorLocation();
}
