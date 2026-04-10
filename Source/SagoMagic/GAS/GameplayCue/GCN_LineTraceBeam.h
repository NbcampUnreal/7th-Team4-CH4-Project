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

	//TODO: 스태프 무기 추가시 스태프 끝 소켓 이름으로 변경
	//현재는 스태프가 없으므로 몸 중앙에 임시 부착
	UPROPERTY(EditDefaultsOnly, Category = "Beam")
	FName AttachSocketName = TEXT("pelvis");

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BeamNiagaraComponent;

	TWeakObjectPtr<AActor> TargetActor;

	float BeamRange = 0.f;

	// OnActive/ WhileActive 공유 초기화 로직
	void InitializeBeam(AActor* MyTarget, const FGameplayCueParameters& Parameters);
	void UpdateBeam();

	bool HasAnyTeamTag(AActor* Actor) const;
};
