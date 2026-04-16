// AGCN_LineTraceChain.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GCN_LineTraceChain.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class SAGOMAGIC_API AGCN_LineTraceChain : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	
	AGCN_LineTraceChain();
	
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	virtual void Tick(float DeltaTime) override;
	
protected:
	/** BP에서 NS_Beam 에셋 할당 (GCN_LineTraceBeam과 동일 에셋 재사용 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	TObjectPtr<UNiagaraSystem> ChainBeamNiagaraSystem;

	/** 시전자 빔 시작 소켓 (0번 구간 시작점) */
	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	FName AttachSocketName = TEXT("Staff_Tip");

	/** 체인 탐색 반경 배수 */
	UPROPERTY(EditDefaultsOnly, Category = "Chain")
	float ChainSearchRadiusMultiplier = 0.8f;
	
	float ChainSearchRadius = 800.f;

private:
	/** 각 체인 구간 빔 컴포넌트 배열 (MaxChainCount개) */
	UPROPERTY()
	TArray<TObjectPtr<UNiagaraComponent>> ChainBeamComponents;

	TWeakObjectPtr<AActor> OwnerActor;
	float BeamRange = 0.f;
	int32 MaxChainCount = 3;  // GA에서 NormalizedMagnitude로 전달받음

	// OnActive / WhileActive 공유 초기화
	void InitializeChain(AActor* MyTarget, const FGameplayCueParameters& Parameters);

	// 매 프레임: 체인 포인트 계산 → 각 구간 컴포넌트 갱신
	void UpdateChain();

	// 반경 내 가장 가까운 적 탐색 (SphereOverlapActors 사용)
	bool FindNearestEnemy(const FVector& Origin, const TArray<AActor*>& ExcludeActors,
						  AActor*& OutEnemy) const;

	bool HasAnyTeamTag(AActor* Actor) const;
	
	FVector GetAttachSocketLocation(ACharacter* Character) const;
};
