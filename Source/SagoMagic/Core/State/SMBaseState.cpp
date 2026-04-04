#include "SMBaseState.h"

#include "Core/SMGameMode.h"
#include "Core/SMStateMachine.h"

void USMBaseState::Initialize(USMStateMachine* InStateMachine)
{
    StateMachine = InStateMachine;
}

void USMBaseState::Enter()
{
    ASMGameState* GameState = GetGameState();
    if (GameState)
    {
        //TODO 은서 : GameState에서 해줘야하는 것
    }
}

void USMBaseState::Tick(float DeltaTime)
{
}

void USMBaseState::Exit()
{
}

void USMBaseState::ChangeState(EGameState NewState)
{
    if (StateMachine)
    {
        StateMachine->ChangeState(NewState);
    }
}

ASMGameMode* USMBaseState::GetGameMode() const
{
    if (StateMachine == nullptr)
    {
        return nullptr;
    }
    return StateMachine->GetOwner();
}

ASMGameState* USMBaseState::GetGameState() const
{
    ASMGameMode* GameMode = GetGameMode();
    if (GameMode == nullptr)
    {
        return nullptr;
    }
    return GameMode->GetGameState<ASMGameState>();

}
