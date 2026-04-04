// SMPlayerController.cpp


#include "SMPlayerController.h"
#include "Core/SMPlayerState.h"
#include "Core/SessionSubsystem/SMLobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/SessionUI/SMLobbyWidget.h"

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

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	FString MapName = UGameplayStatics::GetCurrentLevelName(World, true);
	
	if (MapName.Contains(LobbyMapName) == true)
	{
		ShowLobbyWidget();
	}
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

void ASMPlayerController::ShowLobbyWidget()
{
	if (IsValid(LobbyWidgetClass) == false) return;
	
	LobbyWidgetInstance = CreateWidget<USMLobbyWidget>(this, LobbyWidgetClass);
	if (IsValid(LobbyWidgetInstance) == false) return;

	LobbyWidgetInstance->AddToViewport();
	LobbyWidgetInstance->LobbySetup();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(LobbyWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}
