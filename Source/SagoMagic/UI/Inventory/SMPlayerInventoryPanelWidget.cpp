#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

#include "GameFramework/PlayerController.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "GameplayTags/Message/SMMessageTag.h"

#include "UI/Inventory/SMInventoryGridWidget.h"
#include "UI/Inventory/SMSkillInventoryWidget.h"
#include "UI/Inventory/SMQuickSlotBarWidget.h"
#include "UI/Inventory/SMInventoryContextMenuWidget.h"
#include "UI/Inventory/SMItemHoverInfoWidget.h"

USMPlayerInventoryPanelWidget::USMPlayerInventoryPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , MainInventoryGridWidget(nullptr)
	  , SkillInventoryWidget(nullptr)
	  , QuickSlotBarWidget(nullptr)
	  , ContextMenuWidget(nullptr)
	  , ItemHoverInfoWidget(nullptr)
	  , ContextMenuScreenPosition(FVector2D::ZeroVector)
{
}

void USMPlayerInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USMPlayerInventoryPanelWidget::NativeDestruct()
{
	UnregisterInventoryMessageListeners();
	Super::NativeDestruct();
}

void USMPlayerInventoryPanelWidget::InitializePanelWidget(USMInventoryComponent* InInventoryComponent)
{
	UnregisterInventoryMessageListeners();
	InventoryComponent = InInventoryComponent;
	InitializeChildWidgets();
	RegisterInventoryMessageListeners();
	RefreshPanel();
}

void USMPlayerInventoryPanelWidget::RegisterInventoryMessageListeners()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MainInventoryUpdatedListenerHandle = MessageSubsystem.RegisterListener<FSMInventoryUpdatedMessage>(
		SMMessageTag::Inventory_MainContainerUpdated,
		this,
		&ThisClass::HandleInventoryUpdatedMessage);
	SkillContainerUpdatedListenerHandle = MessageSubsystem.RegisterListener<FSMInventoryUpdatedMessage>(
		SMMessageTag::Inventory_SkillContainerUpdated,
		this,
		&ThisClass::HandleInventoryUpdatedMessage);
	SkillSummaryUpdatedListenerHandle = MessageSubsystem.RegisterListener<FSMSkillSummaryUpdatedMessage>(
		SMMessageTag::Inventory_SkillSummaryUpdated,
		this,
		&ThisClass::HandleSkillSummaryUpdatedMessage);
}

void USMPlayerInventoryPanelWidget::UnregisterInventoryMessageListeners()
{
	if (MainInventoryUpdatedListenerHandle.IsValid())
	{
		MainInventoryUpdatedListenerHandle.Unregister();
	}

	if (SkillContainerUpdatedListenerHandle.IsValid())
	{
		SkillContainerUpdatedListenerHandle.Unregister();
	}

	if (SkillSummaryUpdatedListenerHandle.IsValid())
	{
		SkillSummaryUpdatedListenerHandle.Unregister();
	}
}

void USMPlayerInventoryPanelWidget::HandleInventoryUpdatedMessage(
	FGameplayTag InChannel,
	const FSMInventoryUpdatedMessage& InMessage)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	APlayerState* OwningPlayerState = OwningPlayerController->GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr || InMessage.GetOwningPlayerState() != OwningPlayerState)
	{
		return;
	}

	if (InChannel == SMMessageTag::Inventory_MainContainerUpdated)
	{
		ApplySelectedSkillState();
		RefreshMainInventoryWidget();
		return;
	}

	if (InChannel == SMMessageTag::Inventory_SkillContainerUpdated &&
		SkillInventoryWidget != nullptr &&
		SkillInventoryWidget->GetContainerId().IsValid() &&
		SkillInventoryWidget->GetContainerId() == InMessage.GetContainerId())
	{
		RefreshSkillInventoryWidget();
	}
}

void USMPlayerInventoryPanelWidget::HandleSkillSummaryUpdatedMessage(
	FGameplayTag InChannel,
	const FSMSkillSummaryUpdatedMessage& InMessage)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	APlayerState* OwningPlayerState = OwningPlayerController->GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr || InMessage.GetOwningPlayerState() != OwningPlayerState)
	{
		return;
	}

	ApplySelectedSkillState();
	RefreshMainInventoryWidget();
}

void USMPlayerInventoryPanelWidget::RefreshPanel()
{
	if (HoveredItemInstanceId.IsValid() && (InventoryComponent == nullptr || InventoryComponent->HasItem(HoveredItemInstanceId) == false))
	{
		HideHoveredItemInfo();
	}
	else if (ItemHoverInfoWidget != nullptr && HoveredItemInstanceId.IsValid())
	{
		ItemHoverInfoWidget->RefreshItemInfo();
	}

	if (ContextMenuWidget != nullptr)
	{
		const FGuid& ContextMenuItemInstanceId = ContextMenuWidget->GetItemInstanceId();
		if (ContextMenuItemInstanceId.IsValid() &&
			(InventoryComponent == nullptr || InventoryComponent->HasItem(ContextMenuItemInstanceId) == false))
		{
			CloseContextMenu();
		}
	}

	ApplySelectedSkillState();
	RefreshMainInventoryWidget();
	RefreshSkillInventoryWidget();
	RefreshQuickSlotBarWidget();

	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::SelectSkill(const FGuid& InSkillInstanceId)
{
	OpenSkillInventory(InSkillInstanceId);
}

void USMPlayerInventoryPanelWidget::ClearSelectedSkill()
{
	CloseSkillInventory();
}

void USMPlayerInventoryPanelWidget::OpenSkillInventory(const FGuid& InSkillInstanceId)
{
	if (SelectedSkillInstanceId == InSkillInstanceId)
	{
		ApplySelectedSkillState();
		BP_OnPanelRefreshed();
		return;
	}

	SelectedSkillInstanceId = InSkillInstanceId;
	ApplySelectedSkillState();
	RefreshSkillInventoryWidget();
	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::CloseSkillInventory()
{
	if (SelectedSkillInstanceId.IsValid() == false)
	{
		ApplySelectedSkillState();
		BP_OnPanelRefreshed();
		return;
	}

	SelectedSkillInstanceId.Invalidate();
	ApplySelectedSkillState();
	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::RefreshMainInventoryWidget()
{
	if (HoveredItemInstanceId.IsValid() && (InventoryComponent == nullptr || InventoryComponent->HasItem(HoveredItemInstanceId) == false))
	{
		HideHoveredItemInfo();
	}
	else if (ItemHoverInfoWidget != nullptr && HoveredItemInstanceId.IsValid())
	{
		ItemHoverInfoWidget->RefreshItemInfo();
	}

	if (ContextMenuWidget != nullptr)
	{
		const FGuid& ContextMenuItemInstanceId = ContextMenuWidget->GetItemInstanceId();
		if (ContextMenuItemInstanceId.IsValid() &&
			(InventoryComponent == nullptr || InventoryComponent->HasItem(ContextMenuItemInstanceId) == false))
		{
			CloseContextMenu();
		}
	}

	if (MainInventoryGridWidget == nullptr)
	{
		return;
	}

	MainInventoryGridWidget->RefreshGrid();
}

void USMPlayerInventoryPanelWidget::RefreshSkillInventoryWidget()
{
	ApplySelectedSkillState();

	if (SkillInventoryWidget != nullptr)
	{
		if (SelectedSkillInstanceId.IsValid() == false || SkillInventoryWidget->GetContainerId().IsValid() == false)
		{
			return;
		}

		SkillInventoryWidget->RefreshGrid();
	}
}

void USMPlayerInventoryPanelWidget::RefreshQuickSlotBarWidget()
{
	if (QuickSlotBarWidget != nullptr)
	{
		QuickSlotBarWidget->RefreshQuickSlotBar();
	}
}

bool USMPlayerInventoryPanelWidget::RequestRotateCurrentDraggedItem()
{
	if (MainInventoryGridWidget != nullptr && MainInventoryGridWidget->GetActiveDragDropOperation() != nullptr)
	{
		MainInventoryGridWidget->RequestRotateDraggedItem();
		return true;
	}

	if (SkillInventoryWidget != nullptr && SkillInventoryWidget->GetActiveDragDropOperation() != nullptr)
	{
		SkillInventoryWidget->RequestRotateDraggedItem();
		return true;
	}

	return false;
}

void USMPlayerInventoryPanelWidget::ClearActiveDragState()
{
	if (MainInventoryGridWidget != nullptr)
	{
		MainInventoryGridWidget->ClearActiveDragState();
	}

	if (SkillInventoryWidget != nullptr)
	{
		SkillInventoryWidget->ClearActiveDragState();
	}
}

void USMPlayerInventoryPanelWidget::SetActiveDragGrid(
	USMInventoryGridWidget* InActiveGrid,
	USMInventoryDragDropOperation* InOperation)
{
	if (MainInventoryGridWidget != nullptr && MainInventoryGridWidget != InActiveGrid)
	{
		MainInventoryGridWidget->ClearActiveDragState();
	}

	if (SkillInventoryWidget != nullptr && SkillInventoryWidget != InActiveGrid)
	{
		SkillInventoryWidget->ClearActiveDragState();
	}

	if (InActiveGrid != nullptr)
	{
		InActiveGrid->SetActiveDragOperation(InOperation);
	}
}

void USMPlayerInventoryPanelWidget::SetHoveredItem(const FGuid& InItemInstanceId)
{
	if (HoveredItemInstanceId == InItemInstanceId)
	{
		return;
	}

	HoveredItemInstanceId = InItemInstanceId;
	BP_OnHoveredItemChanged();
}

void USMPlayerInventoryPanelWidget::ClearHoveredItem()
{
	if (HoveredItemInstanceId.IsValid() == false)
	{
		return;
	}

	HoveredItemInstanceId.Invalidate();
	BP_OnHoveredItemChanged();
}

void USMPlayerInventoryPanelWidget::OpenContextMenuForItem(const FGuid& InItemInstanceId, FVector2D InScreenPosition)
{
	if (ContextMenuWidget == nullptr)
	{
		return;
	}

	ContextMenuScreenPosition = InScreenPosition;
	ContextMenuWidget->InitializeContextMenu(InItemInstanceId, InventoryComponent);
	BP_OnContextMenuStateChanged();
}

void USMPlayerInventoryPanelWidget::CloseContextMenu()
{
	ContextMenuScreenPosition = FVector2D::ZeroVector;

	if (ContextMenuWidget != nullptr)
	{
		ContextMenuWidget->InitializeContextMenu(FGuid(), InventoryComponent);
	}

	BP_OnContextMenuStateChanged();
}

void USMPlayerInventoryPanelWidget::ShowHoveredItemInfo(const FGuid& InItemInstanceId, FVector2D InScreenPosition)
{
	if (InItemInstanceId.IsValid() == false)
	{
		HideHoveredItemInfo();
		return;
	}

	const bool bHoveredItemChanged = HoveredItemInstanceId != InItemInstanceId;
	HoveredItemInstanceId = InItemInstanceId;

	if (ItemHoverInfoWidget != nullptr)
	{
		if (ItemHoverInfoWidget->GetItemInstanceId() == InItemInstanceId && ItemHoverInfoWidget->IsShowingItemInfo())
		{
			ItemHoverInfoWidget->UpdateScreenPosition(InScreenPosition);
		}
		else
		{
			ItemHoverInfoWidget->ShowItemInfo(InItemInstanceId, InScreenPosition);
		}
	}

	if (bHoveredItemChanged)
	{
		BP_OnHoveredItemChanged();
	}
}

void USMPlayerInventoryPanelWidget::HideHoveredItemInfo()
{
	const bool bWasHovered = HoveredItemInstanceId.IsValid();
	HoveredItemInstanceId.Invalidate();

	if (ItemHoverInfoWidget != nullptr)
	{
		ItemHoverInfoWidget->HideItemInfo();
	}

	if (bWasHovered)
	{
		BP_OnHoveredItemChanged();
	}
}

void USMPlayerInventoryPanelWidget::InitializeChildWidgets()
{
	FGuid MainInventoryContainerId;
	if (InventoryComponent != nullptr)
	{
		MainInventoryContainerId = InventoryComponent->GetMainInventory().ContainerId;
	}

	if (MainInventoryGridWidget != nullptr)
	{
		MainInventoryGridWidget->InitializeGridWidget(MainInventoryContainerId, InventoryComponent);
	}

	if (SkillInventoryWidget != nullptr)
	{
		SkillInventoryWidget->SetInventoryComponent(InventoryComponent);
	}

	if (QuickSlotBarWidget != nullptr)
	{
		QuickSlotBarWidget->InitializeQuickSlotBarWidget(InventoryComponent);
	}

	if (ContextMenuWidget != nullptr)
	{
		ContextMenuWidget->SetInventoryComponent(InventoryComponent);
		ContextMenuWidget->SetOwningPanelWidget(this);
		ContextMenuWidget->SetItemInstanceId(FGuid());
	}

	if (ItemHoverInfoWidget != nullptr)
	{
		ItemHoverInfoWidget->InitializeHoverInfoWidget(InventoryComponent);
	}

	HideHoveredItemInfo();
	CloseContextMenu();
	ApplySelectedSkillState();
}

void USMPlayerInventoryPanelWidget::ApplySelectedSkillState()
{
	if (SkillInventoryWidget == nullptr)
	{
		return;
	}

	if (InventoryComponent == nullptr)
	{
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	if (SelectedSkillInstanceId.IsValid() == false)
	{
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(SelectedSkillInstanceId, SkillData) == false)
	{
		SelectedSkillInstanceId.Invalidate();
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	SkillInventoryWidget->ChangeTargetSkill(SelectedSkillInstanceId, SkillData.InternalContainerId);
}
