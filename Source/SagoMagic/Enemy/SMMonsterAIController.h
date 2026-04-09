#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "SMMonsterAIController.generated.h"

UCLASS()
class SAGOMAGIC_API ASMMonsterAIController : public AAIController
{
    GENERATED_BODY()
public:
    ASMMonsterAIController();
    void StopAttackTimer();

protected:
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    
public:

    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    /** MonsterBase의 GiveDefaultAbilities() 완료 후 호출 **/
    void StartAttackTimer();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UAIPerceptionComponent> PerceptionComp;

    UPROPERTY(EditAnywhere, Category = "AI")
    TObjectPtr<class UBehaviorTree> BTAsset;

    UPROPERTY(EditAnywhere, Category = "AI")
    TObjectPtr<class UBlackboardData> BBAsset;

    UPROPERTY()
    TObjectPtr<class UBlackboardComponent> BlackboardComp;

    /** 플레이어와 이 거리 이하일 때 공격을 시도 **/
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float AttackRange = 150.0f;

    /** 공격 쿨다운 (초) **/
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float AttackCooldown = 2.0f;

private:
    /** 매 틱 대신 타이머로 거리 체크  **/
    FTimerHandle AttackCheckTimerHandle;

    void CheckAttackRange();
};
