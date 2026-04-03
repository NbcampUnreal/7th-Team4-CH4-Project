#include "Enemy/SMMonsterAIController.h"

ASMMonsterAIController::ASMMonsterAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ASMMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    //나중에 behaviorTree
    //MoveToLocation(FVector::ZeroVector);
    // 블랙보드 에셋이 유효한지 확인 후 초기화
    if (BBAsset)
    {
        // 1. 블랙보드 데이터를 초기화하여 사용 준비
        UseBlackboard(BBAsset, BlackboardComp);

        // 2. 비헤이비어 트리 실행 (내부적으로 블랙보드를 참조함)
        RunBehaviorTree(BTAsset);
    }
}
