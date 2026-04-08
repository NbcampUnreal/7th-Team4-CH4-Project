// GCN_LineTraceBeam.cpp


#include "GAS/GameplayCue/GCN_LineTraceBeam.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "GameplayTags/Character/SMSkillTag.h"

AGCN_LineTraceBeam::AGCN_LineTraceBeam()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // OnActive에서 수동 활성화

	bAutoDestroyOnRemove = true; //OnRemove 후 자동 삭제

	GameplayCueTag = SMSkillTag::GameplayCue_Skill_LineTrace_Beam;

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
	//UE_LOG(LogTemp,Warning,TEXT("BeamNiagaraComponent::InitializeBeam()"));
	if (IsValid(BeamNiagaraComponent) && BeamNiagaraComponent->IsActive())
	{
		return;
	}

	TargetActor = MyTarget;
	BeamRange = Parameters.RawMagnitude; //GA_LineTrace에서 RangeCm으로 전달한 값

	if (IsValid(BeamNiagaraSystem) == false) return;
	BeamNiagaraComponent->SetAsset(BeamNiagaraSystem);

	//TODO: 스태프 무기 추가시 변경필요
	ACharacter* Character = Cast<ACharacter>(MyTarget);
	if (IsValid(Character) == false || IsValid(Character->GetMesh()) == false) return;
	BeamNiagaraComponent->AttachToComponent(
		Character->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocketName
	);
	
	BeamNiagaraComponent->Activate(true);
	SetActorTickEnabled(true);
}

void AGCN_LineTraceBeam::UpdateBeam()
{
	if (IsValid(BeamNiagaraComponent) == false || TargetActor.IsValid() == false) return;
	
	ACharacter* Character = Cast<ACharacter>(TargetActor.Get());
	if (IsValid(Character) == false) return;
	
	AController* Controller = Character->GetController();
	if (IsValid(Controller) == false) return;
	
	const FVector Origin = Character->GetActorLocation();
	const FVector AimDirection = Controller->GetControlRotation().Vector();
	const FVector TraceEnd = Origin + AimDirection * BeamRange;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TargetActor.Get());
	
	FVector BeamEndPoint;
	if (GetWorld()->LineTraceSingleByChannel(HitResult,Origin,TraceEnd,ECC_Pawn,Params))
	{
		BeamEndPoint = HitResult.ImpactPoint;
	}
	else
	{
		BeamEndPoint = TraceEnd;
	}
	
	//Niagara USER파라미터 "BeamEnd" 갱신
	BeamNiagaraComponent->SetVariableVec3(TEXT("BeamEnd"),BeamEndPoint);
}
