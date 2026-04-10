#include "SMGameState.h"

#include "SagoMagic.h"
#include "DataManager/SMAsyncDataManager.h"
#include "Net/UnrealNetwork.h"
#include "Wave/SMWaveManagerSubsystem.h"
#include "Character/SMPlayerController.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "GameplayTags/UI/SMUITag.h"

ASMGameState::ASMGameState()
{
}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(ASMGameState, CurrentState);
    DOREPLIFETIME(ASMGameState, ReplicatedWaveIndex);
    DOREPLIFETIME(ASMGameState, BuildTimeRemaining);
    DOREPLIFETIME(ASMGameState, CombatTimeRemaining);
    DOREPLIFETIME(ASMGameState, AssetsToLoad);
    DOREPLIFETIME(ASMGameState, AssetsLoadSerial);
    DOREPLIFETIME(ASMGameState, MaxBuildTime);
    DOREPLIFETIME(ASMGameState, MaxCombatTime);
    
}

void ASMGameState::PostNetInit()
{
    Super::PostNetInit();
}

void ASMGameState::SetCurrentState(EGameState NewState)
{
    //서버에서만 변경 (GameMode->StateMachine->여기 호출)
    CurrentState = NewState;
    //서버 측 구독자에게도 브로드캐스트
    OnGameStateChanged.Broadcast(NewState);
}

void ASMGameState::SetAssetsToLoad(const TArray<FPrimaryAssetId>& InAssets)
{
    if (!HasAuthority())
    {
        ensureMsgf(false, TEXT("ASMGameState::SetAssetsToLoad must be called on the server only."));
        return;
    }
    AssetsToLoad = InAssets;
    AssetsLoadSerial++;
    ForceNetUpdate();
}

void ASMGameState::SetBuildTimeRemaining(int32 WaveIndex, float TimeRemaining, float InMaxTime)
{
    ReplicatedWaveIndex = WaveIndex;
    BuildTimeRemaining = TimeRemaining;
    MaxBuildTime         = InMaxTime;
}

void ASMGameState::SetCombatInfo(int32 WaveIndex, float TimeRemaining, float InMaxTime)
{
    ReplicatedWaveIndex = WaveIndex;
    CombatTimeRemaining = TimeRemaining;
    MaxCombatTime        = InMaxTime;
    
    BroadcastWaveMsg(EWaveUIState::Inprogress, WaveIndex, TimeRemaining, InMaxTime);
}

void ASMGameState::OnRep_CurrentState()
{
    OnGameStateChanged.Broadcast(CurrentState);
}

void ASMGameState::OnRep_WaveIndex()
{
    //TODO 현 : 현재 WaveIndex 브로드케스트
    EWaveUIState CurrentUIState = (CurrentState == EGameState::Build) ? EWaveUIState::Preparing : EWaveUIState::Inprogress;
    
    float CurrentTimeRemaining = (CurrentState == EGameState::Build) ? BuildTimeRemaining : CombatTimeRemaining;
    float CurrentMaxTime = (CurrentState == EGameState::Build) ? MaxBuildTime : MaxCombatTime;

    BroadcastWaveMsg(CurrentUIState, ReplicatedWaveIndex, CurrentTimeRemaining, CurrentMaxTime);
}

void ASMGameState::OnRep_BuildTimeRemaining()
{
    //TODO 현 : 현재 정비 시간 브로드케스트
    SM_LOG(this, LogSM, Log, TEXT("[Build] 클라이언트 수신 - WaveIndex=%d TimeRemaining=%.1f"),
        ReplicatedWaveIndex, BuildTimeRemaining);
    
    // 정비 상태, 동기화된 시간들로 UI 업데이트
    BroadcastWaveMsg(EWaveUIState::Preparing, ReplicatedWaveIndex, BuildTimeRemaining, MaxBuildTime);
}

void ASMGameState::OnRep_CombatTimeRemaining()
{
    //TODO 현 : 현재 전투 시간 브로드케스트
    SM_LOG(this, LogSM, Log, TEXT("[Combat] 클라이언트 수신 - WaveIndex=%d TimeRemaining=%.1f"),
        ReplicatedWaveIndex, CombatTimeRemaining);
    
    // 전투 진행 중 상태, 동기화된 시간들로 UI 업데이트
    BroadcastWaveMsg(EWaveUIState::Inprogress, ReplicatedWaveIndex, CombatTimeRemaining, MaxCombatTime);
}

void ASMGameState::OnRep_AssetsToLoad()
{
    UE_LOG(LogTemp, Log, TEXT("[GameState] OnRep_AssetsToLoad 발동 - Serial=%d, Assets=%d개"),
        AssetsLoadSerial, AssetsToLoad.Num());
    
    if (AssetsToLoad.IsEmpty()) return;
    
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    if (!AM) return;
    
    AM->LoadAssetsByIDWithBundles(AssetsToLoad, TArray<FName>{"Client"},
        FOnAssetLoadComplete::CreateLambda([this]()
        {
            //이 월드에 존재하는 모든 PlayerController를 순회
            for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                UE_LOG(LogTemp, Log, TEXT("[GameState] 클라이언트 에셋 로드 완료 - PC 탐색"));
                ASMPlayerController* PC = Cast<ASMPlayerController>(It->Get());
                
                if (PC && PC->IsLocalController())
                {
                    UE_LOG(LogTemp, Log, TEXT("[GameState] ServerNotifyClientLoadComplete 호출"));
                    PC->ServerNotifyClientLoadComplete();
                    return;
                }
            }
            
        })
    );
}

void ASMGameState::BroadcastWaveMsg(EWaveUIState InState, int32 InWaveIndex, float InTimeRemaining, float InMaxTime)
{
    FWaveMsg Msg;
    Msg.State         = InState; // 현재 웨이브의 진행 상태
    Msg.WaveIndex     = InWaveIndex; // 현재 웨이브 번호
    Msg.TimeRemaining = InTimeRemaining; // 해당 페이즈의 남은 시간
    Msg.MaxTime       = InMaxTime; // 해당 페이즈 전체 시간 - 프로그래스바 비율 계산을 위한 용도!

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(SMUITag::Event_Wave, Msg);
}
