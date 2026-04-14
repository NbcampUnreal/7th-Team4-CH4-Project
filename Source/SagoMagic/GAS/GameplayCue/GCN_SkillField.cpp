#include "GAS/GameplayCue/GCN_SkillField.h"
#include "NiagaraComponent.h"


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

	// 스킬 범위에 맞게 이펙트 스케일 조정
	// NormalizedMagnitude = RangeCm, NiagaraBaseRadiusCm = Scale_All 1.0일 때의 기본 반경
	if (Parameters.NormalizedMagnitude > 0.f && NiagaraBaseRadiusCm > 0.f)
	{
		const float ScaleValue = Parameters.NormalizedMagnitude / NiagaraBaseRadiusCm;
		FieldNiagaraComponent->SetVariableFloat(FName("User.Scale_All"), ScaleValue);
	}

	FieldNiagaraComponent->Activate(true);

	// 지속시간 후 파티클 신규 스폰 중단 → 기존 파티클은 자연 소멸
	const float Lifetime = Parameters.RawMagnitude > 0.f ? Parameters.RawMagnitude : 5.f;
	GetWorldTimerManager().SetTimer(
		FadeoutTimerHandle,
		this,
		&AGCN_SkillField::StartFadeout,
		Lifetime,
		false
	);
	// 파티클이 다 사라질 시간까지 액터 유지 제발
	SetLifeSpan(Lifetime + FadeoutDuration);

	return true;
}

bool AGCN_SkillField::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 타이머가 아직 안 됐으면 즉시 페이드아웃 시작
	GetWorldTimerManager().ClearTimer(FadeoutTimerHandle);
	StartFadeout();

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGCN_SkillField::StartFadeout()
{
	if (IsValid(FieldNiagaraComponent))
	{
		// 신규 스폰 중단 — 기존 파티클은 수명대로 자연 소멸
		FieldNiagaraComponent->Deactivate();
	}
}
