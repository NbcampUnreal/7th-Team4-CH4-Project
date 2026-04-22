#include "GAS/GameplayCue/GCN_SkillField.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Core/DataManager/SMSoundManager.h"
#include "GAS/Abilities/SkillActor/SMASkillField.h"


AGCN_SkillField::AGCN_SkillField()
{
	PrimaryActorTick.bCanEverTick = false;
	// GC - Cleanup 부분에서 Auto Destroy on Remove 부분이 BP에서 덮어쓸 수 있기때문에 코드에서 따로 체크해줌.
	bAutoDestroyOnRemove = false;
	// 장판이 여러개 깔릴 수 있으므로 독립된 인스턴스 보장
	bUniqueInstancePerInstigator = true;

	FieldNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FieldNiagaraComponent"));
	SetRootComponent(FieldNiagaraComponent);
	FieldNiagaraComponent->SetAutoActivate(false);
}

bool AGCN_SkillField::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);

	if (!FieldNiagaraSystem) return false;

	ASMASkillField* SourceField = Cast<ASMASkillField>(Parameters.EffectContext.GetEffectCauser());
	const float Lifetime = SourceField ? SourceField->GetFieldDuration() : 5.0f;
	const float RangCm = SourceField ? SourceField->GetFieldRangeCm() : 0.f;

	// 장판 스폰 위치로 이동
	const FVector Origin = Parameters.EffectContext.GetOrigin();
	const FVector FieldLocation = Origin.IsNearlyZero()
		                              ? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector)
		                              : Origin;
	SetActorLocation(FieldLocation);


	FieldNiagaraComponent->SetAsset(FieldNiagaraSystem);

	// 스킬 범위에 맞게 이펙트 스케일 조정
	// NormalizedMagnitude = RangeCm, NiagaraBaseRadiusCm = Scale_All 1.0일 때의 기본 반경
	if (RangCm > 0.f && NiagaraBaseRadiusCm > 0.f)
	{
		FieldNiagaraComponent->SetVariableFloat(FName("User.Scale_All"), RangCm / NiagaraBaseRadiusCm);
	}

	FieldNiagaraComponent->Activate(true);

	// 루프 사운드 재생
	if (IsValid(LoopAudioComponent) == false)
	{
		USMSoundManager* SM = USMSoundManager::Get(this);
		if (IsValid(SM) == true)
		{
			LoopAudioComponent = SM->PlaySoundLoopAttached(TEXT("FieldAttack"), FieldNiagaraComponent);
		}
	}

	// 지속시간 후 파티클 신규 스폰 중단 → 기존 파티클은 자연 소멸
	GetWorldTimerManager().SetTimer(
		FadeoutTimerHandle,
		this,
		&AGCN_SkillField::StartFadeout,
		Lifetime,
		false
	);
	// 파티클이 다 사라질 시간까지 액터 유지 제발
	GetWorldTimerManager().SetTimer(
		ReturnToPoolTimerHandle,
		this,
		&AGCN_SkillField::GameplayCueFinishedCallback,
		Lifetime + FadeoutDuration,
		false);

	return true;
}

bool AGCN_SkillField::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	return OnActive_Implementation(MyTarget, Parameters);
}

bool AGCN_SkillField::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 타이머가 아직 안 됐으면 즉시 페이드아웃 시작
	GetWorldTimerManager().ClearTimer(FadeoutTimerHandle);
	StartFadeout();

	// 루프 사운드 페이드아웃
	USMSoundManager* SM = USMSoundManager::Get(this);
	if (IsValid(SM) == true)
	{
		SM->StopSoundLoop(LoopAudioComponent, SoundFadeOutDuration);
	}
	LoopAudioComponent = nullptr;
	
	GetWorldTimerManager().SetTimer(
		ReturnToPoolTimerHandle,
		this,
		&AGCN_SkillField::GameplayCueFinishedCallback,
		FadeoutDuration,
		false);

	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGCN_SkillField::GameplayCueFinishedCallback()
{
	Destroy();
}

void AGCN_SkillField::StartFadeout()
{
	if (IsValid(FieldNiagaraComponent))
	{
		// 신규 스폰 중단 — 기존 파티클은 수명대로 자연 소멸
		FieldNiagaraComponent->Deactivate();
	}
}
