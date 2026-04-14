#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "SMMonsterAIController.generated.h"

//공격 대상 타입
UENUM(BlueprintType)
enum class EMonsterAttackTargetType : uint8
{
    None,
    BaseCamp,
    Building,
    Player
};

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

    /** 현재 공격 대상 타입 (GA_MonsterAttackBase에서 읽음) **/
    EMonsterAttackTargetType CurrentTargetType = EMonsterAttackTargetType::BaseCamp;

    /** 현재 공격 중인 실제 대상 (GA_MonsterRangedAttack이 발사 방향 계산에 사용) **/
    UPROPERTY()
    TObjectPtr<AActor> CurrentAttackTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UAIPerceptionComponent> PerceptionComp;

    UPROPERTY(EditAnywhere, Category = "AI")
    TObjectPtr<class UBehaviorTree> BTAsset;

    UPROPERTY(EditAnywhere, Category = "AI")
    TObjectPtr<class UBlackboardData> BBAsset;

    UPROPERTY()
    TObjectPtr<class UBlackboardComponent> BlackboardComp;

    /** 공격 판정 거리 — 이 범위 안에 들어온 대상만 공격 **/
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float AttackRange = 150.0f;

    /** 공격 쿨다운 (초) **/
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float AttackCooldown = 2.0f;

    /** 구조물 감지 반경 (경로 위 장애물 탐색) **/
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float BuildingDetectRadius = 300.0f;

private:
    FTimerHandle AttackCheckTimerHandle;

    /** 매 쿨다운마다 호출: 이동 타겟 갱신 + 공격 범위 체크 **/
    void UpdateTargetAndTryAttack();

    /**
     * AttackRange 이내에서 공격할 대상을 찾음
     * 우선순위: 건물 > 플레이어 > BaseCamp
     * "지금 때릴 수 있는 놈"만 반환 — 이동 목표와 무관
     */
    AActor* FindAttackableTarget();

    /** AttackRange 이내의 건물 **/
    AActor* FindBuildingInRange();

    /** AttackRange 이내의 가장 가까운 플레이어 **/
    AActor* FindNearestPlayerInRange();

    /** 가장 가까운 살아있는 BaseCamp **/
    AActor* FindBaseCamp();
};
