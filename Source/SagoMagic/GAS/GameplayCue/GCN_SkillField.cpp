#include "GAS/GameplayCue/GCN_SkillField.h"

#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

AGCN_SkillField::AGCN_SkillField()
{
	PrimaryActorTick.bCanEverTick = false;
	// GC - Cleanup 부분에서 Auto Destroy on Remove 부분이 BP에서 덮어쓸 수 있기때문에 코드에서 따로 체크해줌.
	bAutoDestroyOnRemove = false;

	FieldNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FieldNiagaraComponent"));
	FieldNiagaraComponent->SetupAttachment(RootComponent);
	FieldNiagaraComponent->SetAutoActivate(false);
}

bool AGCN_SkillField::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);

	if (!FieldNiagaraSystem) return false;

	// 장판 스폰 위치로 이동
	const FVector FieldLocation = Parameters.Location.IsNearlyZero()
		? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector)
		: Parameters.Location;

	SetActorLocation(FieldLocation);

	FieldNiagaraComponent->SetAsset(FieldNiagaraSystem);
	FieldNiagaraComponent->Activate(true);

	//5초후 액터 자동 소멸되게 
	const float Lifetime = Parameters.RawMagnitude > 0.f ? Parameters.RawMagnitude : 5.f;
	SetLifeSpan(Lifetime);

	return true;
}

bool AGCN_SkillField::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	if (IsValid(FieldNiagaraComponent))
	{
		FieldNiagaraComponent->Deactivate();
	}

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}
