#include "SMGameState.h"

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
