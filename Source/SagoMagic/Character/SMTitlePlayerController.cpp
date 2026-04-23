// ASMTitlePlayerController.cpp


#include "SMTitlePlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SessionUI/SMTitleWidget.h"

void ASMTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() == false) return;

	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI) == false) return;
	
	ShowMainWidget();
}

void ASMTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ASMTitlePlayerController::SetPendingServerAddress(const FString& Address)
{
	PendingServerAddress = Address;

	UGameplayStatics::OpenLevel(GetWorld(), FName(*PendingServerAddress), true);
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
