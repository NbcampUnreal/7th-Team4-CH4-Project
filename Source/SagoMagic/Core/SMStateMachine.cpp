#include "SMStateMachine.h"
#include "SMGameMode.h"
#include "SMGameState.h"
#include "DataManager/SMSyncDataManager.h"
#include "State/SMBaseState.h"
#include "State/SMBuildState.h"
#include "State/SMCombatState.h"
#include "State/SMResultState.h"

void USMStateMachine::Initialize(ASMGameMode* InOwner)
{
    Owner = InOwner;
    
    if (USMSyncDataManager* DM = USMSyncDataManager::Get(InOwner->GetWorld()))
    {
        MaxWaveCount = DM->GetWaveCount();
        UE_LOG(LogTemp, Log, TEXT("[StateMachine] MaxWaveCount: %d"), MaxWaveCount);
    }
    
    RegisterStatus();
    //첫 번째 시작 부분
    ApplyState(EGameState::Build);
}

void USMStateMachine::Tick(float DeltaTime)
{
    if (CurrentState)
        CurrentState->Tick(DeltaTime);
}

void USMStateMachine::ChangeState(EGameState NextState)
{
    //이미 해당 State면 무시 (BaseCamp 파괴 + Wave클리어 동시 발동 방어)
    if (CurrentStateType == NextState) return;
    
    if (CurrentStateType == EGameState::Combat && NextState == EGameState::Build)
    {
        CurrentWaveIndex++;
    }
    ApplyState(NextState);
}

void USMStateMachine::RegisterStatus()
{
    //auto Register : Register라는 이름의 변수
    // = [&] : 바깥 변수(StateCache,this등)를 참조로 캡처
    //USMBaseState* State : State 포인터를 인자로 받음
    auto Register = [&](USMBaseState* State)
    {
        State->Initialize(this);
        StateCache.Add(State->GetStateType(), State);
    };

    Register(NewObject<USMBuildState>(this));
    Register(NewObject<USMCombatState>(this));
    Register(NewObject<USMResultState>(this));
}

void USMStateMachine::ApplyState(EGameState NextState)
{
    // 현재 실행중인 State가 있으면 먼저 종료 처리
    if (CurrentState)
        CurrentState->Exit();

    //Found = 포인터의 포인터
    //*Found = 실제 State 인스턴스
    TObjectPtr<USMBaseState>* Found = StateCache.Find(NextState);
    if (!Found || !(*Found))
    {
        UE_LOG(LogTemp,Warning, TEXT("StateMachine State 없음"));
        return;
    }
    CurrentState = *Found;
    CurrentStateType = NextState;

    if (Owner)
    {
        if (ASMGameState* GS = Owner->GetGameState<ASMGameState>())
            GS->SetCurrentState(NextState);
    }
    CurrentState->Enter();
}
