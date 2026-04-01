#include "Enemy/SMMonsterAIController.h"

ASMMonsterAIController::ASMMonsterAIController()
{
}

void ASMMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    //나중에 behaviorTree
    MoveToLocation(FVector::ZeroVector);
}
