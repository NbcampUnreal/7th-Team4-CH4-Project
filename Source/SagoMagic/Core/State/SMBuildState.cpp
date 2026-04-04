#include "SMBuildState.h"
#include "Core/SMStateMachine.h"

void USMBuildState::Enter()
{
    Super::Enter();

    Elapsed = 0.f;
}

void USMBuildState::Tick(float DeltaTime)
{
    Elapsed += DeltaTime;

    if (Elapsed >= Duration)
    {
        UE_LOG(LogTemp, Log, TEXT("BuildState -> Combat"));
        ChangeState(EGameState::Combat);
    }
}

void USMBuildState::Exit()
{
    Elapsed = 0;
    Super::Exit();
}
