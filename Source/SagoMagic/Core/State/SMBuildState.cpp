#include "SMBuildState.h"

#include "SagoMagic.h"
#include "Core/SMGameMode.h"
#include "Core/SMStateMachine.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Core/SMGameState.h"

void USMBuildState::Enter()
{
    Super::Enter();

    Elapsed = 0.f;
    bReadyForCombat = false;
    
    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    CachedGameState = StateMachine->GetOwner()->GetWorld()->GetGameState<ASMGameState>();
    CurrentWaveIndex = StateMachine->GetCurrentWaveIndex();
    USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this);
    if (!WM) return;
    WM->OnReadyForCombat.BindLambda([this]()
    {
       bReadyForCombat = true;
        UE_LOG(LogTemp, Log, TEXT("[BuildState] PreSpawn 완료 - Combat 준비됨"));
    });
    WM->PreSpawnForWave(WaveIndex);
}

void USMBuildState::Tick(float DeltaTime)
{
    Elapsed += DeltaTime;
    SyncElapsed += DeltaTime;
    
    if (SyncElapsed >= 1.0f)
    {
        SyncElapsed = 0.f;
        if (CachedGameState)
        {
            SM_LOG(this, LogSM, Log, TEXT("[Build] WaveIndex=%d TimeRemaining=%.1f 서버→GameState 세팅"),
            StateMachine->GetCurrentWaveIndex(), Duration-Elapsed);
            CachedGameState->SetBuildTimeRemaining(CurrentWaveIndex,
                FMath::Max(0.f,Duration-Elapsed));
        }
    }
    
    if (Elapsed >= Duration)
    {
        if (bReadyForCombat)
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
