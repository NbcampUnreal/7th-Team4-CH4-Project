#include "SMGameState.h"
#include "Net/UnrealNetwork.h"

ASMGameState::ASMGameState()
{
}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASMGameState, CurrentState);
}

void ASMGameState::PostNetInit()
{
    Super::PostNetInit();
}

void ASMGameState::SetCurrentState(EGameState NewState)
{
    //서버에서만 변경 (GameMode->StateMachine->여기 호출)
    CurrentState = NewState;
    //서버 측 구독자에게도 브로드캐스트
    OnGameStateChanged.Broadcast(NewState);
}

void ASMGameState::OnRep_CurrentState()
{
    OnGameStateChanged.Broadcast(CurrentState);
}
