#include "SMGameFlowSubSystem.h"

#include "SMGameState.h"

void USMGameFlowSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USMGameFlowSubSystem::Deinitialize()
{
    //command 정리
    if (CurrentCommand)
        CurrentCommand->Exit(nullptr);
    CurrentCommand.Reset();

    Super::Deinitialize();
}

void USMGameFlowSubSystem::Tick(float DeltaTime)
{
}

TStatId USMGameFlowSubSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USMGameFlowSubSystem, STATGROUP_Tickables);
}

void USMGameFlowSubSystem::AdvanceCommand()
{
    if (CurrentCommand)
        CurrentCommand->Exit(GetWorld()->GetAuthGameMode<ASMGameMode>());

    TSharedPtr<IGamePhaseCommand> Next;
    if (CommandQueue.Dequeue(Next))
    {
        CurrentCommand = Next;
        CurrentCommand->OnCompleted.BindUObject(
            this, &USMGameFlowSubSystem::AdvanceCommand);
        CurrentCommand->Enter(GetWorld()->GetAuthGameMode<ASMGameMode>());
    }

}
