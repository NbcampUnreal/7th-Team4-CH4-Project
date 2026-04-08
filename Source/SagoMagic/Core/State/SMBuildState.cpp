#include "SMBuildState.h"
#include "Core/SMStateMachine.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"

void USMBuildState::Enter()
{
    Super::Enter();

    Elapsed = 0.f;
    bReadForCombat = false;
    
    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    
    USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this);
    if (!WM) return;
    
    WM->OnReadyForCombat.BindLambda([this]()
    {
       bReadForCombat = true;
        UE_LOG(LogTemp, Log, TEXT("[BuildState] PreSpawn 완료 - Combat 준비됨"));
    });
    WM->PreSpawnForWave(WaveIndex);
}

void USMBuildState::Tick(float DeltaTime)
{
    Elapsed += DeltaTime;

    if (Elapsed >= Duration)
    {
        if (bReadForCombat)
        {
            UE_LOG(LogTemp, Log, TEXT("BuildState -> Combat"));
            ChangeState(EGameState::Combat);    
        }
        //TODO 현 : loading 동그라미 생성 시작 위치!
    }
}

void USMBuildState::Exit()
{
    Elapsed = 0;
    Super::Exit();
}
