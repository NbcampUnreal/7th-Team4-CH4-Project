#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterAIController.h"

ASMMonsterBase::ASMMonsterBase()
{
    AIControllerClass = ASMMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

}

void ASMMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASMMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Health 변수를 복제 대상으로 등록
    DOREPLIFETIME(ASMMonsterBase, Health);
}

void ASMMonsterBase::OnRepHealth()
{

}

