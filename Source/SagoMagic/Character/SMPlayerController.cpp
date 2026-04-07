// SMPlayerController.cpp


#include "SMPlayerController.h"

#include "SagoMagic.h"
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

void ASMPlayerController::ClientRPC_ShowDeathUI_Implementation()
{
	
	// TODO: 현님이 UI완성하면 호출
	SetInputMode(FInputModeGameAndUI());
	
	SM_LOG(this, LogSM, Log, TEXT("사망 UI"));
}

void ASMPlayerController::ClientRPC_HideDeathUI_Implementation()
{
	
	// TODO: 현님이 UI완성하면 호출
	SetInputMode(FInputModeGameOnly());
	
	SM_LOG(this, LogSM, Log, TEXT("부활시 사망 UI 숨김"));
}

void ASMPlayerController::ClientRPCArrivedAtGameLevel_Implementation()
{
	//입력 모드 전환 (기존 OnArrivedAtGameLevel 로직)
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
	
	ASMPlayerState* PS = GetPlayerState<ASMPlayerState>();
	if (!PS) return;
	
	UE_LOG(LogTemp, Log, TEXT("[GameLevel] 플레이어 도착 - 이름:%s / Host:%d"),
		*PS->GetPlayerName(), PS->GetIsHost());
	//TODO 현 : 게임 HUD 생성, 캐릭터 선택 등 여기서 처리
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
