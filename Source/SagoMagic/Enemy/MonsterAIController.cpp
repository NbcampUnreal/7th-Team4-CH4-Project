#include "Enemy/MonsterAIController.h"

AMonsterAIController::AMonsterAIController()
{
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    //나중에 behaviorTree
    MoveToLocation(FVector::ZeroVector);
}
