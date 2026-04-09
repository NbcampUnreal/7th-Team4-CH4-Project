#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_ProjectileHit.generated.h"

class UNiagaraSystem;

UCLASS()
class SAGOMAGIC_API UGCN_ProjectileHit : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	/** BP에서 히트 나이아가라 에셋 지정 - FreeMagic - 필드마법쪽으로 해주기*/
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> HitNiagaraSystem;

	//이펙트 색상 파라미터 - Niagara User Parameter 이름과 일치해야 함
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Color")
	FLinearColor Color_REF = FLinearColor(0.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Color")
	FLinearColor Color_Smoke = FLinearColor(0.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Color")
	FLinearColor Color_Sparks1 = FLinearColor(0.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Color")
	FLinearColor Color_Spiral1 = FLinearColor(0.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Color")
	FLinearColor Color_Trail = FLinearColor(0.f, 1.f, 1.f, 1.f);

	/** 이펙트 스케일 파라미터 */
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Scale")
	float Scale_All = 1.f;
};
