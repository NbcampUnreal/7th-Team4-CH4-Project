#include "SMBuildState.h"

#include "SagoMagic.h"
#include "Core/SMGameMode.h"
#include "Core/SMStateMachine.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "GameplayTags/UI/SMUITag.h"
#include "UI/SMGameplayMessages.h"
#include "Core/SMGameState.h"

void USMBuildState::Enter()
{
    Super::Enter();

    Elapsed = 0.f;
    bReadyForCombat = false;
    
    int32 WaveIndex = StateMachine->GetCurrentWaveIndex();
    CachedGameState = StateMachine->GetOwner()->GetWorld()->GetGameState<ASMGameState>();
    CurrentWaveIndex = StateMachine->GetCurrentWaveIndex();
    
    // 정비 시작 알림
    BroadcastNotification(FText::Format(FText::FromString(TEXT("정비 단계 시작")),
        FText::AsNumber(CurrentWaveIndex)), 2.0f);
    
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
            CachedGameState->SetBuildTimeRemaining(CurrentWaveIndex,
                FMath::Max(0.f,Duration-Elapsed),
                Duration);
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
    // 정비 종료 알림
    BroadcastNotification(FText::Format(FText::FromString(TEXT("정비 단계 종료")),
        FText::AsNumber(CurrentWaveIndex)), 1.5f);
    
    Elapsed = 0;
    Super::Exit();
}

void USMBuildState::BroadcastNotification(const FText& InMessage, float InDuration)
{
    if (CachedGameState)
    {
        // 캐싱된 GameState를 통해 Multicast 함수 호출
        CachedGameState->Multicast_BroadcastNotification(InMessage, InDuration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildState] CachedGameState가 null입니다! 알림을 보낼 수 없습니다."));
    }
}
