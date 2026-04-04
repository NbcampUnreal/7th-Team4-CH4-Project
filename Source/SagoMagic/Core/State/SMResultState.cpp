#include "SMResultState.h"

#include "Core/SMStateMachine.h"

void USMResultState::Enter()
{
    Super::Enter();

    //승리 조건 : WaveIndex가 Max에 도달한 상태로 Enter
    //패배 조건 : NotifyDefeat()를 통해 강제 진입
    bool bIsVictory = StateMachine->GetCurrentWaveIndex() >= 3;

    UE_LOG(LogTemp, Log, TEXT("[ResultState] Enter - %s"), bIsVictory ? TEXT("승리") : TEXT("패배"));

    //TODO 은서 : 결과 UI Multicast RPC 호출
    //예 : GM->MulticastShowResult(bIsVictory);
}

void USMResultState::Tick(float DeltaTime)
{
    // 재도전 대기, 로비 복귀 등 추후 구현
}

void USMResultState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("[ResultState] Exit"));
}

void USMResultState::NotifyDefeat()
{
    UE_LOG(LogTemp, Log, TEXT("[ResultState] 베이스캠프 파괴 -> Result(패배)"));
    ChangeState(EGameState::Result);
}
