// SMPlayerController.cpp


#include "SMPlayerController.h"

#include "SagoMagic.h"
#include "Core/SMPlayerState.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Core/SessionSubsystem/SMLobbyGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "InputAction.h"
#include "Core/SMGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/SMInventoryRootWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"
#include "UI/SessionUI/SMLobbyWidget.h"
#include "UI/SMGameplayMessages.h" 
#include "UI/SMHUD.h"
#include "UI/SMHUDManager.h"

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

	ApplyControllerMappingContext();

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	FString MapName = UGameplayStatics::GetCurrentLevelName(World, true);
	
	if (MapName.Contains(LobbyMapName) == true)
	{
		ShowLobbyWidget();
	}
}

void ASMPlayerController::ClientRPC_ShowDeathUI_Implementation(float RespawnTime)
{
	// TODO: 현님이 UI완성하면 호출
	SM_LOG(this, LogSM, Log, TEXT("사망 UI 표시 - %.1f초 후 부활"), RespawnTime);
	
	// GameplayMessage 대신 HUD 직접 접근 (태그 미등록 문제 우회)
	
	// PlayerController -> HUD -> HUDManager -> PlayerDeathWidget 순서로 접근
	if (ASMHUD* HUD = Cast<ASMHUD>(GetHUD()))
	{
		if (USMHUDManager* HUDMgr = HUD->GetHUDManager())
		{
			HUDMgr->ShowPlayerDeath(RespawnTime);
		}
	}
	
	// 사망 중 입력 모드 유지 (마우스 커서는 표시)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ASMPlayerController::ClientRPC_HideDeathUI_Implementation()
{
	// TODO: 현님이 UI완성하면 호출
	SM_LOG(this, LogSM, Log, TEXT("부활 - 사망 UI 숨김"));
	
	if (ASMHUD* HUD = Cast<ASMHUD>(GetHUD()))
	{
		if (USMHUDManager* HUDMgr = HUD->GetHUDManager())
		{
			HUDMgr->HidePlayerDeath();
		}
	}
	
	// 게임플레이 입력 모드 복원
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void ASMPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController() == false)
	{
		return;
	}

	ApplyControllerMappingContext();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleInventoryAction != nullptr)
		{
			EnhancedInputComponent->BindAction(
				ToggleInventoryAction,
				ETriggerEvent::Started,
				this,
				&ThisClass::ToggleInventory);
		}

		if (RotateInventoryItemAction != nullptr)
		{
			EnhancedInputComponent->BindAction(
				RotateInventoryItemAction,
				ETriggerEvent::Started,
				this,
				&ThisClass::RotateDraggedInventoryItem);
		}
	}
}

void ASMPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	
	LoginNotify_Implementation();
	
}

void ASMPlayerController::LoginNotify_Implementation()
{
	ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>();	
	if (IsValid(GM) == false) return;
	
	GM->OnPlayerReady(this);
	
}

void ASMPlayerController::ClientRPC_ShowGameResult_Implementation(bool bIsVictory, float InReturnDelay)
{
	SM_LOG(this, LogSM, Log, TEXT("게임 결과 UI - %s"), bIsVictory ? TEXT("승리") : TEXT("패배"));

	if (ASMHUD* HUD = Cast<ASMHUD>(GetHUD()))
	{
		if (USMHUDManager* HUDMgr = HUD->GetHUDManager())
		{
			HUDMgr->ShowGameResult(bIsVictory, InReturnDelay);
		}
	}
}

void ASMPlayerController::ApplyControllerMappingContext()
{
	if (IsLocalPlayerController() == false)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem == nullptr)
	{
		return;
	}

	if (ControllerMappingContext == nullptr)
	{
		return;
	}

	Subsystem->RemoveMappingContext(ControllerMappingContext);
	Subsystem->AddMappingContext(ControllerMappingContext, 0);
}

void ASMPlayerController::ClientRPCArrivedAtGameLevel_Implementation()
{
	//입력 모드 전환 (기존 OnArrivedAtGameLevel 로직)
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(true);

	ApplyControllerMappingContext();

	ASMPlayerState* PS = GetPlayerState<ASMPlayerState>();
	if (!PS) return;

	UE_LOG(LogTemp, Log, TEXT("[GameLevel] 플레이어 도착 - 이름:%s / Host:%d"),
		*PS->GetPlayerName(), PS->GetIsHost());

	//TODO 현 : 게임 HUD 생성, 캐릭터 선택 등 여기서 처리
}

void ASMPlayerController::ServerNotifyClientLoadComplete_Implementation()
{
	if (USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this))
	{
		WM->OnClientLoadComplete();
	}
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

void ASMPlayerController::ServerRPCMoveInventoryItem_Implementation(
	const FGuid& InItemInstanceId,
	const FGuid& InTargetContainerId,
	int32 InGridX,
	int32 InGridY,
	ESMGridRotation InRotation)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->MoveItem(InItemInstanceId, InTargetContainerId, InGridX, InGridY, InRotation);
}

void ASMPlayerController::ServerRPCDropInventoryItem_Implementation(const FGuid& InItemInstanceId)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	AActor* DropSourceActor = GetPawn();
	if (DropSourceActor == nullptr)
	{
		DropSourceActor = GetViewTarget();
	}

	if (DropSourceActor == nullptr)
	{
		DropSourceActor = this;
	}

	FRotator DropRotation = GetControlRotation();
	DropRotation.Pitch = 0.0f;
	DropRotation.Roll = 0.0f;

	if (DropRotation.IsNearlyZero())
	{
		DropRotation = DropSourceActor->GetActorRotation();
		DropRotation.Pitch = 0.0f;
		DropRotation.Roll = 0.0f;
	}

	const FVector DropLocation =
		DropSourceActor->GetActorLocation() +
		(DropRotation.Vector() * 150.0f) +
		FVector(0.0f, 0.0f, 30.0f);

	const FTransform DropTransform(DropRotation, DropLocation, FVector::OneVector);

	InventoryComponent->DropItem(InItemInstanceId, DropTransform);
}

void ASMPlayerController::ServerRPCRemoveInventoryItem_Implementation(const FGuid& InItemInstanceId)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->RemoveItem(InItemInstanceId);
}

void ASMPlayerController::ServerRPCDetachEmbeddedItem_Implementation(const FGuid& InItemInstanceId)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->DetachEmbeddedItem(InItemInstanceId);
}

void ASMPlayerController::ServerRPCSetActiveQuickSlot_Implementation(int32 InSlotIndex)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->SetActiveQuickSlot(InSlotIndex);
}

void ASMPlayerController::ServerRPCEquipSkillToQuickSlot_Implementation(const FGuid& InSkillInstanceId, int32 InSlotIndex)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->EquipSkillToQuickSlot(InSkillInstanceId, InSlotIndex);
}

void ASMPlayerController::ServerRPCUnequipSkillFromQuickSlotToMainInventory_Implementation(
	int32 InSlotIndex,
	int32 InGridX,
	int32 InGridY,
	ESMGridRotation InRotation)
{
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->UnequipSkillFromQuickSlotToMainInventory(InSlotIndex, InGridX, InGridY, InRotation);
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

void ASMPlayerController::RotateDraggedInventoryItem()
{
	if (IsLocalController() == false)
	{
		return;
	}

	if (bIsInventoryVisible == false || InventoryRootWidgetInstance == nullptr)
	{
		return;
	}

	USMPlayerInventoryPanelWidget* CurrentPanelWidget = InventoryRootWidgetInstance->GetCurrentPanelWidget();
	if (CurrentPanelWidget == nullptr)
	{
		return;
	}

	CurrentPanelWidget->RequestRotateCurrentDraggedItem();
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
	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
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

	APlayerState* OwningPlayerState = GetPlayerState<APlayerState>();
	USMInventoryComponent* InventoryComponent = nullptr;
	if (OwningPlayerState != nullptr)
	{
		InventoryComponent = OwningPlayerState->FindComponentByClass<USMInventoryComponent>();
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
