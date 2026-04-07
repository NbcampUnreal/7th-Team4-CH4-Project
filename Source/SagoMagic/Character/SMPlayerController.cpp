// SMPlayerController.cpp


#include "SMPlayerController.h"
#include "Core/SMPlayerState.h"
#include "Core/SessionSubsystem/SMLobbyGameMode.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/SMInventoryRootWidget.h"
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

void ASMPlayerController::ToggleInventory()
{
	if (IsLocalController() == false)
	{
		return;
	}

	InitializeInventoryWidget();

	if (InventoryRootWidgetInstance == nullptr)
	{
		return;
	}

	if (bIsInventoryVisible)
	{
		HideInventoryWidget();
		return;
	}

	ShowInventoryWidget();
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

void ASMPlayerController::InitializeInventoryWidget()
{
	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	if (PlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = PlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	if (InventoryRootWidgetInstance == nullptr)
	{
		if (InventoryRootWidgetClass == nullptr)
		{
			return;
		}

		InventoryRootWidgetInstance = CreateWidget<USMInventoryRootWidget>(this, InventoryRootWidgetClass);
		if (InventoryRootWidgetInstance == nullptr)
		{
			return;
		}

		InventoryRootWidgetInstance->AddToViewport();
		InventoryRootWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (InventoryRootWidgetInstance->GetInventoryComponent() != InventoryComponent)
	{
		InventoryRootWidgetInstance->InitializeRootWidget(InventoryComponent);
	}
}

void ASMPlayerController::ShowInventoryWidget()
{
	if (InventoryRootWidgetInstance == nullptr)
	{
		return;
	}

	APlayerState* PlayerState = GetPlayerState<APlayerState>();
	USMInventoryComponent* InventoryComponent = nullptr;
	if (PlayerState != nullptr)
	{
		InventoryComponent = PlayerState->FindComponentByClass<USMInventoryComponent>();
	}

	if (InventoryComponent != nullptr)
	{
		if (InventoryRootWidgetInstance->GetInventoryComponent() != InventoryComponent)
		{
			InventoryRootWidgetInstance->InitializeRootWidget(InventoryComponent);
		}
		else
		{
			InventoryRootWidgetInstance->RefreshRootWidget();
		}
	}

	InventoryRootWidgetInstance->SetVisibility(ESlateVisibility::Visible);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(InventoryRootWidgetInstance->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	bIsInventoryVisible = true;
}

void ASMPlayerController::HideInventoryWidget()
{
	if (InventoryRootWidgetInstance == nullptr)
	{
		return;
	}

	InventoryRootWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	bIsInventoryVisible = false;
}
