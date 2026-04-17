// GCN_LineTraceBeam.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GCN_LineTraceBeam.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
/**
 * 레이저 빔 발사용 GameplayCueNotify_Actor 입니다
 */
UCLASS()
class SAGOMAGIC_API AGCN_LineTraceBeam : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGCN_LineTraceBeam();

	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	virtual void Tick(float DeltaTime) override;

protected:
	/** BP에서 나이아가라 빔 에셋 할당*/
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	TObjectPtr<UNiagaraSystem> BeamNiagaraSystem;
	
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	FName AttachSocketName = TEXT("Staff_Tip");

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BeamNiagaraComponent;

	TWeakObjectPtr<AActor> TargetActor;

	float BeamRange = 0.f;

	// OnActive/ WhileActive 공유 초기화 로직
	void InitializeBeam(AActor* MyTarget, const FGameplayCueParameters& Parameters);
	void UpdateBeam();

	bool HasAnyTeamTag(AActor* Actor) const;
	
	bool IsAvailableEnemy(AActor* Actor) const;
	
	bool bPenetrate = false;
	
	FVector GetAttachSocketLocation(ACharacter* Character) const;
};
