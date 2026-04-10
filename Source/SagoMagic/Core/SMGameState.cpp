#include "SMGameState.h"

#include "SagoMagic.h"
#include "DataManager/SMAsyncDataManager.h"
#include "Net/UnrealNetwork.h"
#include "Wave/SMWaveManagerSubsystem.h"
#include "Character/SMPlayerController.h"

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

void ASMGameState::SetBuildTimeRemaining(int32 WaveIndex, float TimeRemaining)
{
    ReplicatedWaveIndex = WaveIndex;
    BuildTimeRemaining = TimeRemaining;
}

void ASMGameState::SetCombatInfo(int32 WaveIndex, float TimeRemaining)
{
    ReplicatedWaveIndex = WaveIndex;
    CombatTimeRemaining = TimeRemaining;
}

void ASMGameState::MulticastPreloadClientAssets_Implementation(const TArray<FPrimaryAssetId>& AssetIds)
{
    // 서버는 WaveManagerSubsystem에서 이미 처리
    if (HasAuthority()) return;
    UE_LOG(LogTemp, Log, TEXT("[GameState] 클라이언트 Multicast 수신 - 로드 시작"));
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    if (!AM) return;
    
    AM->LoadAssetsByIDWithBundles(AssetIds, TArray<FName>{"Client"},
        FOnAssetLoadComplete::CreateLambda([this]()
        {
            UE_LOG(LogTemp, Log, TEXT("[GameState] 클라이언트 DataAsset 로드 완료"));
            // PlayerController를 통해 Server RPC 호출
            if (ASMPlayerController* PC = Cast<ASMPlayerController>(GetWorld()->GetFirstPlayerController()))
            {
                PC->ServerNotifyClientLoadComplete();
            }
        })
    );
}

void ASMGameState::OnRep_CurrentState()
{
    OnGameStateChanged.Broadcast(CurrentState);
}

void ASMGameState::OnRep_WaveIndex()
{
    //TODO 현 : 현재 WaveIndex 브로드케스트
    //예시
    // FBuildPhaseMsg Msg;
    // Msg.WaveIndex = ReplicatedWaveIndex;
    // Msg.TimeRemaining = BuildTimeRemaining;
    //
    // UGameplayMessageSubsystem::Get(this).BroadcastMessage(
    //     FGameplayTag::RequestGameplayTag(FName("UI.Event.Wave.Build"), false), Msg
    // );
    
    //GamePlayTag.h에 추가할  Struct는 이런 느낌??
    // USTRUCT(BlueprintType)
    // struct FBuildPhaseMsg
    // {
    //     GENERATED_BODY()
    //
    //     UPROPERTY(BlueprintReadOnly)
    //     int32 WaveIndex = 0;
    //
    //     UPROPERTY(BlueprintReadOnly)
    //     float TimeRemaining = 0.f;  // 빌드 준비 카운트다운
    // };
}

void ASMGameState::OnRep_BuildTimeRemaining()
{
    //TODO 현 : 현재 정비 시간 브로드케스트
    SM_LOG(this, LogSM, Log, TEXT("[Build] 클라이언트 수신 - WaveIndex=%d TimeRemaining=%.1f"),
        ReplicatedWaveIndex, BuildTimeRemaining);
}

void ASMGameState::OnRep_CombatTimeRemaining()
{
    //TODO 현 : 현재 전투 시간 브로드케스트
    SM_LOG(this, LogSM, Log, TEXT("[Combat] 클라이언트 수신 - WaveIndex=%d TimeRemaining=%.1f"),
        ReplicatedWaveIndex, CombatTimeRemaining);
}
