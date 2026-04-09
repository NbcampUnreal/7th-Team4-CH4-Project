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

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> FieldNiagaraComponent;
};
