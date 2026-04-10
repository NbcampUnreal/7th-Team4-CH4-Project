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
    DOREPLIFETIME(ASMGameState, AssetsToLoad);
    DOREPLIFETIME(ASMGameState, AssetsLoadSerial);
    
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
    AssetsToLoad = InAssets;
    AssetsLoadSerial++;
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
                UE_LOG(LogTemp, Error, TEXT("[GameState] 로컬 PC를 찾지 못함!"));
            }
            
        })
    );
}
