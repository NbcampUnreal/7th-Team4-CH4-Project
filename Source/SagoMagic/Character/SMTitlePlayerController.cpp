// ASMTitlePlayerController.cpp


#include "SMTitlePlayerController.h"
#include "OnlineSubsystem.h"
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
}

void ASMTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalController() == true && IsValid(SessionSubsystem) == true )
	{
		UnbindSessionDelegates();	
	}
	
	Super::EndPlay(EndPlayReason);
}

void ASMTitlePlayerController::SetPendingServerAddress(const FString& Address)
{
	PendingServerAddress = Address;
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

void ASMTitlePlayerController::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful == false) return;

	UGameplayStatics::OpenLevel(
		GetWorld(),FName(*PendingServerAddress),true);
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
		GetWorld(),FName(*Address),true);
}

void ASMTitlePlayerController::BindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnCreateSessionComplete.AddDynamic(
		this, &ASMTitlePlayerController::OnCreateSessionComplete);

	SessionSubsystem->OnJoinSessionComplete.AddUObject(
		this, &ASMTitlePlayerController::OnJoinSessionComplete);
}

void ASMTitlePlayerController::UnbindSessionDelegates()
{
	if (!SessionSubsystem) return;

	SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(
		this, &ASMTitlePlayerController::OnCreateSessionComplete);

	SessionSubsystem->OnJoinSessionComplete.RemoveAll(this);
}
