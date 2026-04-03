// SMLobbyWidget.cpp


#include "SMLobbyWidget.h"

#include "OnlineSubsystem.h"
#include "Character/SMPlayerController.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Core/SMPlayerState.h"
#include "Core/SessionSubsystem/SMLobbyGameState.h"
#include "Interfaces/OnlineSessionInterface.h"

void USMLobbyWidget::LobbySetup()
{
    ASMLobbyGameState* GS = GetLobbyGameState();
    if (IsValid(GS) == true)
    {
        GS->OnPlayerSlotChanged.AddDynamic(
            this,&ThisClass::OnPlayerSlotsUpdated);

        UpdateSlotUI(GS->GetPlayerSlots());
    }

    UpdateButtonVisibility();
}

bool USMLobbyWidget::Initialize()
{
    if (Super::Initialize() == false) return false;

    if (IsValid(ReadyButton) == true)
    {
        ReadyButton->OnClicked.AddDynamic(
            this, &ThisClass::OnReadyButtonClicked);
    }
    if (IsValid(StartButton) == true)
    {
        StartButton->OnClicked.AddDynamic(
            this, &ThisClass::OnStartButtonClicked);
    }
    if (IsValid(InviteButton) == true)
    {
        InviteButton->OnClicked.AddDynamic(
            this, &ThisClass::OnInviteButtonClicked);
    }

    return true;
}

void USMLobbyWidget::NativeDestruct()
{
    TearDown();
    Super::NativeDestruct();
}

void USMLobbyWidget::OnReadyButtonClicked()
{
    ASMPlayerController* PC = GetSMPlayerController();
    if (IsValid(PC) == false) return;

    ASMPlayerState* PS = PC->GetPlayerState<ASMPlayerState>();
    if (IsValid(PS) == false) return;

    // Server RPC → LobbyGameMode.SetPlayerReady() 호출
    //PC->ServerRPCSetReady(!PS->GetIsReady());
}

void USMLobbyWidget::OnStartButtonClicked()
{
    StartButton->SetIsEnabled(false);

    // Server RPC → LobbyGameMode.TryStartGame() 호출
    ASMPlayerController* PC = GetSMPlayerController();
    if (IsValid(PC) == true)
    {
        //PC->ServerRPCRequestStartGame();
    }
}

void USMLobbyWidget::OnInviteButtonClicked()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem ) return;

    IOnlineSessionPtr SI = Subsystem->GetSessionInterface();
    if (SI.IsValid() == false) return;

    SI->SendSessionInviteToFriends(
        0, NAME_GameSession, TArray<FUniqueNetIdRef>());
}

void USMLobbyWidget::OnPlayerSlotsUpdated()
{
    ASMLobbyGameState* GS = GetLobbyGameState();
    if (IsValid(GS) == false) return;

    UpdateSlotUI(GS->GetPlayerSlots());
    UpdateButtonVisibility();
}

void USMLobbyWidget::UpdateSlotUI(const TArray<FSMPlayerSlotInfo>& Slots)
{
    if (PlayerSlotBox) return;

    PlayerSlotBox->ClearChildren();

    for (const auto& PlayerSlot : Slots)
    {
        //TODO: 스를 위젯 생성 후 PlayerSlotBox에 추가
    }
}

void USMLobbyWidget::UpdateButtonVisibility()
{
    ASMPlayerController* PC = GetSMPlayerController();
    if (!PC) return;

    ASMPlayerState* PS = PC->GetPlayerState<ASMPlayerState>();
    if (!PS) return;

    // 방장만 StartButton 표시
    if (StartButton)
    {
        //StartButton->SetVisibility(
        //    PS->GetIsHost() ?
        //    ESlateVisibility::Visible :
        //     ESlateVisibility::Collapsed);
    }
}

void USMLobbyWidget::TearDown()
{
    // GameState 델리게이트 해제
    ASMLobbyGameState* GS = GetLobbyGameState();
    if (GS)
    {
        GS->OnPlayerSlotChanged.RemoveDynamic(
            this, &ThisClass::OnPlayerSlotsUpdated);
    }

    // InputMode 초기화 제거 — PlayerController가 담당
}

ASMPlayerController* USMLobbyWidget::GetSMPlayerController() const
{
    return Cast<ASMPlayerController>(GetOwningPlayer());
}

ASMLobbyGameState* USMLobbyWidget::GetLobbyGameState() const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    return World->GetGameState<ASMLobbyGameState>();
}
