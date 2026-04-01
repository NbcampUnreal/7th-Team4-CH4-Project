#include "Enemy/MonsterBase.h"
#include "Enemy/MonsterAIController.h"

AMonsterBase::AMonsterBase()
{
    AIControllerClass = AMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Health 변수를 복제 대상으로 등록
    DOREPLIFETIME(AMonsterBase, Health);
}

void AMonsterBase::OnRepHealth()
{

}
