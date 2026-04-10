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

    /** 현재 공격 대상 타입(GA_MonsterAttackBase에서 읽음) **/
    EMonsterAttackTargetType CurrentTargetType = EMonsterAttackTargetType::BaseCamp;

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
    // 구조물 감지 반경 (경로 위 장애물 탐색)
    // TODO: 구조물 클래스 완성 후 이 값을 에디터에서 조정
    UPROPERTY(EditAnywhere, Category = "AI|Attack")
    float StructureDetectRadius = 300.0f;
private:
    /** 매 틱 대신 타이머로 거리 체크  **/
    FTimerHandle AttackCheckTimerHandle;

    void CheckAttackRange();

    /**
    * 공격 우선순위에 따라 최적 타겟을 선정
    * 우선순위: 경로 위 건축물 > 범위 내 플레이어 > 없으면 basecamp
    */
    AActor* FindBestTarget();

    /**
     * 몬스터 전방 경로에 공격 가능한 건물 체크
     * TODO: ISMStructureInterface 또는 ASMStructureBase 완성 후 구현 채우기
     */
    AActor* FindBuildingOnPath();

    /**
     * AttackRange 이내의 가장 가까운 플레이어 
     */
    AActor* FindNearestPlayerInRange();

};
