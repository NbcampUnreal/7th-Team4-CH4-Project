// ASMTitlePlayerController.cpp


#include "SMTitlePlayerController.h"
#include "OnlineSubsystem.h"
#include "Core/SMGameInstance.h"
#include "Core/SessionSubsystem/SMSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SessionUI/SMTitleWidget.h"

void ASMTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false) return;

	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI) == false) return;

	SessionSubsystem = GI->GetSubsystem<USMSessionSubsystem>();
	if (IsValid(SessionSubsystem) == false) return;

	BindSessionDelegates();

	ShowMainWidget();

	USMGameInstance* SMGameInst = Cast<USMGameInstance>(GI);
	if (IsValid(SMGameInst) == true && SMGameInst->bWasLobbyFull == true)
	{
		if (IsValid(MainWidgetInstance) == true)
		{
			MainWidgetInstance->ShowLobbyFullMessage();
		}
		SMGameInst->bWasLobbyFull = false;
	}
}

void ASMTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController() == true && IsValid(SessionSubsystem) == true)
	{
		UnbindSessionDelegates();
	}

	Super::EndPlay(EndPlayReason);
}

void ASMTitlePlayerController::TravelToCheck(const FString& Address)
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(*Address), true);
}

void ASMTitlePlayerController::ShowMainWidget()
{
	if (IsValid(MainWidgetClass) == false) return;

	MainWidgetInstance = CreateWidget<USMTitleWidget>(this, MainWidgetClass);
	if (IsValid(MainWidgetInstance) == false) return;

	MainWidgetInstance->AddToViewport();
	MainWidgetInstance->MenuSetup();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(MainWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ASMTitlePlayerController::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success) return;

	TravelToServer();
}

void ASMTitlePlayerController::TravelToServer()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	IOnlineSessionPtr SI = Subsystem->GetSessionInterface();
	if (SI.IsValid() == false) return;

	FString Address;
	SI->GetResolvedConnectString(NAME_GameSession, Address);
	if (Address.IsEmpty() == true) return;

	UGameplayStatics::OpenLevel(
		GetWorld(), FName(*Address), true);
}

void ASMTitlePlayerController::BindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnJoinSessionComplete.AddUObject(
		this, &ASMTitlePlayerController::OnJoinSessionComplete);
}

void ASMTitlePlayerController::UnbindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnJoinSessionComplete.RemoveAll(this);
}
