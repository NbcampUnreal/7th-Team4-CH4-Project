// SMPlayerController.cpp


#include "SMPlayerController.h"

#include "OnlineSubsystem.h"
#include "Core/SMPlayerState.h"
#include "Core/SessionSubsystem/SMLobbyGameMode.h"
#include "Core/SessionSubsystem/SMSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SessionUI/SMLobbyWidget.h"
#include "UI/SessionUI/SMMainWidget.h"

void ASMPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetShowMouseCursor(true);

	FInputModeGameAndUI InputMode;

	// 마우스 가두기
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);


	SetInputMode(InputMode);

	//네트워크 관련 initialize
	if (IsLocalController() == false) return;

	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI) == true)
	{
		SessionSubsystem = GI->GetSubsystem<USMSessionSubsystem>();
	}

	BindSessionDelegates();

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	FString MapName = UGameplayStatics::GetCurrentLevelName(World, true);

	if (MapName.Contains(MainMapName) == true)
	{
		ShowMainWidget();
	}
	else if (MapName.Contains(LobbyMapName) == true)
	{
		ShowLobbyWidget();
	}
}

void ASMPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController() == true)
	{
		UnbindSessionDelegates();
	}
	Super::EndPlay(EndPlayReason);
}

void ASMPlayerController::SetPendingServerAddress(const FString& Address)
{
	PendingServerAddress = Address;
}

void ASMPlayerController::OnArrivedAtGameLevel()
{
	if (IsLocalController() == false) return;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);

	//TODO: L_Play전용 HUD 생성시 여기서 생성
}

void ASMPlayerController::ServerRPCSetReady_Implementation(bool bReady)
{
	ASMLobbyGameMode* GM = GetWorld()->GetAuthGameMode<ASMLobbyGameMode>();
	if (IsValid(GM) == false) return;

	GM->SetPlayerReady(this, bReady);
}

void ASMPlayerController::ServerRPCRequestStartGame_Implementation()
{
	ASMPlayerState* PS = GetPlayerState<ASMPlayerState>();
	if (!PS || !PS->GetIsHost()) return;

	ASMLobbyGameMode* GM = GetWorld()->GetAuthGameMode<ASMLobbyGameMode>();
	if (IsValid(GM) == false) return;

	GM->TryStartGame();
}

void ASMPlayerController::ShowMainWidget()
{
	if (IsValid(MainWidgetClass) == false) return;

	USMMainWidget* MainWidget = CreateWidget<USMMainWidget>(this, MainWidgetClass);
	if (IsValid(MainWidget) == false) return;

	MainWidget->AddToViewport();
	MainWidget->MenuSetup();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ASMPlayerController::ShowLobbyWidget()
{
	if (IsValid(LobbyWidgetClass) == false) return;

	USMLobbyWidget* LobbyWidget = CreateWidget<USMLobbyWidget>(this, LobbyWidgetClass);
	if (IsValid(LobbyWidget) == false) return;

	LobbyWidget->AddToViewport();
	LobbyWidget->LobbySetup();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ASMPlayerController::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful == false) return;

	ClientTravel(PendingServerAddress, TRAVEL_Absolute);
}

void ASMPlayerController::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success) return;

	TravelToServer();
}

void ASMPlayerController::TravelToServer()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SI = Subsystem->GetSessionInterface();
	if (SI.IsValid() == false) return;

	FString Address;
	SI->GetResolvedConnectString(NAME_GameSession, Address);
	if (Address.IsEmpty() == true) return;

	ClientTravel(Address, TRAVEL_Absolute);
}

void ASMPlayerController::BindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnCreateSessionComplete.AddDynamic(
		this, &ThisClass::ASMPlayerController::OnCreateSessionComplete);

	SessionSubsystem->OnJoinSessionComplete.AddUObject(
		this, &ASMPlayerController::OnJoinSessionComplete);
}

void ASMPlayerController::UnbindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(
		this, &ThisClass::ASMPlayerController::OnCreateSessionComplete);

	SessionSubsystem->OnJoinSessionComplete.RemoveAll(this);
}
