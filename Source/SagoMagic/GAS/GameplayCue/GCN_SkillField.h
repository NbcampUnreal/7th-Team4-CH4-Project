#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GCN_SkillField.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;


UCLASS()
class SAGOMAGIC_API AGCN_SkillField : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGCN_SkillField();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	/** BP에서 장판 나이아가라 에셋 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> FieldNiagaraSystem;

	/** 이펙트 페이드아웃 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float FadeoutDuration = 2.0f;

	/** Niagara Scale_All = 1.0 일 때의 이펙트 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float NiagaraBaseRadiusCm = 380.f;

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> FieldNiagaraComponent;

	// 페이드아웃 시작 타이머
	FTimerHandle FadeoutTimerHandle;

	// 파티클 신규 스폰 중단 기존 파티클은 자연스럽게 소멸
	void StartFadeout();
};
