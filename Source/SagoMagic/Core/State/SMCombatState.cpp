#include "SMCombatState.h"

#include "SagoMagic.h"
#include "Core/SMGameMode.h"
#include "Core/SMStateMachine.h"
#include "Core/SMGameState.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Data/SMWaveData.h"

void USMCombatState::Enter()
{
    Super::Enter();
    Elapsed = 0.f;

    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    UE_LOG(LogTemp, Log, TEXT("[CombatState] Enter - WaveIndex : %d"),WaveIndex);
    
    UWorld* World = StateMachine->GetOwner()->GetWorld();
    CachedGameState   = World->GetGameState<ASMGameState>();
    CachedWaveManager = USMWaveManagerSubsystem::Get(this);
    
    ASMGameMode* GM = GetGameMode();
    if (!GM) return;

    //WaveCleared 이벤트 바인딩
    GM->OnWaveCleared.BindUObject(this, &USMCombatState::OnWaveCleared);

    USMSyncDataManager* DM = USMSyncDataManager::Get(GM->GetWorld());
    if (DM)
    {
        FSMWaveData WaveData = DM->GetWaveData(WaveIndex);
        Duration = WaveData.MaintenanceTime;
        UE_LOG(LogTemp, Log, TEXT("[CombatState] 제한 시간 설정 : %.1f초"),Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatState] DataManager 없음 - 기본값 %.1f초 사용"), Duration);
    }
    if (USMWaveManagerSubsystem* WM = GM->GetWorld()->GetSubsystem<USMWaveManagerSubsystem>())
    {
        WM->StartWave(WaveIndex);
    }
}

void USMCombatState::OnWaveCleared()
{
    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    UE_LOG(LogTemp, Log, TEXT("[CombatState] 웨이브 %d 클리어"),WaveIndex);

    //마지막 웨이브(보스전) 클리어 -> 승리
    if (WaveIndex >= MaxWaveCount - 1)
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatState] 보스 처치 -> Result(승리)"));
        ChangeState(EGameState::Result);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[CombatState] -> Build 전환 (WaveIndex : %d -> %d)"), WaveIndex, WaveIndex);
        ChangeState(EGameState::Build);
    }
}

void USMCombatState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Elapsed += DeltaTime;
    SyncElapsed += DeltaTime;
    
    if (SyncElapsed >= 1.0f)
    {
        SyncElapsed = 0.f;

        if (CachedGameState)
        {
            SM_LOG(this, LogSM, Log, TEXT("[Combat] WaveIndex=%d TimeRemaining=%.1f 서버→GameState 세팅"),
            StateMachine->GetCurrentWaveIndex(), Duration - Elapsed);
            CachedGameState->SetCombatInfo(
                StateMachine->GetCurrentWaveIndex(),
                FMath::Max(0.f, Duration - Elapsed)
            );
        }
    }
    
    if (Elapsed >= Duration)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CombatState] 제한 시간 초과 (%.1f) -> 패배"),Duration);
        ChangeState(EGameState::Result);
    }
    //TODO 은서 : 몬스터가 다 죽었을 때도 체크
    //TODO 은서 : 베이스 캠프 HP 0일때 체크
}

void USMCombatState::Exit()
{
    ASMGameMode* GM = GetGameMode();
    if (GM)
        GM->OnWaveCleared.Unbind();

    UE_LOG(LogTemp, Log, TEXT("[CombatState] Exit"));

}
