#include "SMCombatState.h"

#include "Core/SMGameMode.h"
#include "Core/SMStateMachine.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Data/SMWaveData.h"

void USMCombatState::Enter()
{
    Super::Enter();
    Elapsed = 0.f;

    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    UE_LOG(LogTemp, Log, TEXT("[CombatState] Enter - WaveIndex : %d"),WaveIndex);

    ASMGameMode* GM = GetGameMode();
    if (!GM) return;

    //WaveCleared 이벤트 바인딩
    GM->OnWaveCleared.BindUObject(this, &USMCombatState::OnWaveCleared);

    if (USMWaveManagerSubsystem* WM = GM->GetWorld()->GetSubsystem<USMWaveManagerSubsystem>())
    {
        UDataTable* DataTable = WM->GetWaveDataTable();
        if (DataTable)
        {
            TArray<FSMWaveData*> Rows;
            DataTable->GetAllRows<FSMWaveData>(TEXT(""),Rows);

            if (Rows.IsValidIndex(WaveIndex))
            {
                Duration = Rows[WaveIndex]->MaintenanceTime;
                UE_LOG(LogTemp, Log, TEXT("[CombatState] 제한 시간 설정: %.1f초"), Duration);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[CombatState] WaveIndex %d 범위 초과 - 기본값 %.1f초 사용"),
                    WaveIndex, Duration);
            }
        }
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
        UE_LOG(LogTemp, Log, TEXT("[CombatState] -> Build 전환 (WaveIndex : %d -> %d)"), WaveIndex, WaveIndex + 1);
        ChangeState(EGameState::Build);
    }
}

void USMCombatState::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    Elapsed += DeltaTime;

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
