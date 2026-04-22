#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "GameplayTags/Message/SMMessageTag.h"
#include "GameplayTags/UI/SMUITag.h"

#include "UI/Inventory/SMInventoryGridWidget.h"
#include "UI/Inventory/SMSkillInventoryWidget.h"
#include "UI/Inventory/SMQuickSlotBarWidget.h"
#include "UI/Inventory/SMInventoryContextMenuWidget.h"
#include "UI/Inventory/SMItemHoverInfoWidget.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/SMNotificationWidget.h"

USMPlayerInventoryPanelWidget::USMPlayerInventoryPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , MainInventoryGridWidget(nullptr)
	  , SkillInventoryWidget(nullptr)
	  , QuickSlotBarWidget(nullptr)
	  , ContextMenuWidget(nullptr)
	  , ItemHoverInfoWidget(nullptr)
	  , InventoryNotificationWidget(nullptr)
	  , ContextMenuScreenPosition(FVector2D::ZeroVector)
	  , ActiveDragPreviewOperation(nullptr)
	  , ActiveDragPreviewWidget(nullptr)
{
}

void USMPlayerInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	EnsureFloatingWidgetsCreated();
	HideHoveredItemInfo();
	CloseContextMenu();
}

void USMPlayerInventoryPanelWidget::NativeDestruct()
{
	ClearActiveDragPreview();

	if (ContextMenuWidget != nullptr)
	{
		ContextMenuWidget->RemoveFromParent();
		ContextMenuWidget = nullptr;
	}

	if (ItemHoverInfoWidget != nullptr)
	{
		ItemHoverInfoWidget->RemoveFromParent();
		ItemHoverInfoWidget = nullptr;
	}

	UnregisterInventoryMessageListeners();
	Super::NativeDestruct();
}

void USMPlayerInventoryPanelWidget::EnsureFloatingWidgetsCreated()
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	if (ContextMenuWidget == nullptr && ContextMenuWidgetClass != nullptr)
	{
		ContextMenuWidget = CreateWidget<USMInventoryContextMenuWidget>(OwningPlayerController, ContextMenuWidgetClass);
		if (ContextMenuWidget != nullptr)
		{
			ContextMenuWidget->AddToViewport(1000);
			ContextMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
			ContextMenuWidget->SetInventoryComponent(InventoryComponent);
			ContextMenuWidget->SetOwningPanelWidget(this);
			ContextMenuWidget->SetItemInstanceId(FGuid());
		}
	}

	if (ItemHoverInfoWidget == nullptr && ItemHoverInfoWidgetClass != nullptr)
	{
		ItemHoverInfoWidget = CreateWidget<USMItemHoverInfoWidget>(OwningPlayerController, ItemHoverInfoWidgetClass);
		if (ItemHoverInfoWidget != nullptr)
		{
			ItemHoverInfoWidget->AddToViewport(1001);
			ItemHoverInfoWidget->SetIsEnabled(true);
			ItemHoverInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
			ItemHoverInfoWidget->InitializeHoverInfoWidget(InventoryComponent);
		}
	}
}

FVector2D USMPlayerInventoryPanelWidget::ResolveViewportPosition(FVector2D InAbsolutePosition)
{
	FVector2D PixelPosition = InAbsolutePosition;
	FVector2D ViewportPosition = InAbsolutePosition;
	USlateBlueprintLibrary::AbsoluteToViewport(this, InAbsolutePosition, PixelPosition, ViewportPosition);
	return ViewportPosition;
}

FVector2D USMPlayerInventoryPanelWidget::ClampFloatingWidgetToViewport(
	UUserWidget* InFloatingWidget,
	FVector2D InViewportPosition)
{
	if (InFloatingWidget == nullptr)
	{
		return InViewportPosition;
	}

	ForceLayoutPrepass();
	InFloatingWidget->ForceLayoutPrepass();

	float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
	if (ViewportScale <= 0.0f)
	{
		ViewportScale = 1.0f;
	}

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this) / ViewportScale;
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
	{
		return InViewportPosition;
	}

	FVector2D FloatingWidgetSize = InFloatingWidget->GetDesiredSize();
	const FVector2D CachedWidgetSize = InFloatingWidget->GetCachedGeometry().GetLocalSize();
	if (CachedWidgetSize.X > 0.0f && CachedWidgetSize.Y > 0.0f)
	{
		FloatingWidgetSize = CachedWidgetSize;
	}

	FVector2D ClampedPosition = InViewportPosition;

	if (ClampedPosition.X + FloatingWidgetSize.X > ViewportSize.X)
	{
		ClampedPosition.X = InViewportPosition.X - FloatingWidgetSize.X;
	}

	if (ClampedPosition.Y + FloatingWidgetSize.Y > ViewportSize.Y)
	{
		ClampedPosition.Y = InViewportPosition.Y - FloatingWidgetSize.Y;
	}

	const float MaxX = FMath::Max(ViewportSize.X - FloatingWidgetSize.X, 0.0f);
	const float MaxY = FMath::Max(ViewportSize.Y - FloatingWidgetSize.Y, 0.0f);

	ClampedPosition.X = FMath::Clamp(ClampedPosition.X, 0.0f, MaxX);
	ClampedPosition.Y = FMath::Clamp(ClampedPosition.Y, 0.0f, MaxY);
	return ClampedPosition;
}

void USMPlayerInventoryPanelWidget::InitializePanelWidget(USMInventoryComponent* InInventoryComponent)
{
	UnregisterInventoryMessageListeners();
	InventoryComponent = InInventoryComponent;
	InitializeChildWidgets();
	PreparePanelForOpen();
	RegisterInventoryMessageListeners();
	RefreshPanel();
}

void USMPlayerInventoryPanelWidget::PreparePanelForOpen()
{
	if (InventoryComponent == nullptr)
	{
		SelectedSkillInstanceId.Invalidate();
		ApplySelectedSkillState();
		return;
	}

	FGuid ActiveSkillInstanceId;
	if (InventoryComponent->GetActiveQuickSlotSkillId(ActiveSkillInstanceId))
	{
		SelectedSkillInstanceId = ActiveSkillInstanceId;
	}
	else
	{
		SelectedSkillInstanceId.Invalidate();
	}

	ApplySelectedSkillState();
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
	RefreshMainInventoryWidget();
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
	RefreshMainInventoryWidget();
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
	ClearActiveDragPreview();

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

void USMPlayerInventoryPanelWidget::BeginActiveDragPreview(
	USMInventoryDragDropOperation* InOperation,
	FVector2D InScreenPosition)
{
	if (InOperation == nullptr)
	{
		ClearActiveDragPreview();
		return;
	}

	USMDragItemPreviewWidget* PreviewWidget = InOperation->GetDragPreviewWidget();
	if (PreviewWidget == nullptr)
	{
		ClearActiveDragPreview();
		return;
	}

	if (ActiveDragPreviewWidget != nullptr && ActiveDragPreviewWidget != PreviewWidget)
	{
		ActiveDragPreviewWidget->RemoveFromParent();
	}

	ActiveDragPreviewOperation = InOperation;
	ActiveDragPreviewWidget = PreviewWidget;

	if (PreviewWidget->IsInViewport() == false)
	{
		PreviewWidget->AddToViewport(1000);
	}

	PreviewWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	PreviewWidget->SetIsEnabled(true);
	UpdateActiveDragPreviewPosition(InOperation, InScreenPosition);
}

void USMPlayerInventoryPanelWidget::UpdateActiveDragPreviewPosition(
	USMInventoryDragDropOperation* InOperation,
	FVector2D InScreenPosition)
{
	if (InOperation == nullptr || ActiveDragPreviewWidget == nullptr)
	{
		return;
	}

	if (ActiveDragPreviewOperation != InOperation)
	{
		BeginActiveDragPreview(InOperation, InScreenPosition);
		if (ActiveDragPreviewOperation != InOperation || ActiveDragPreviewWidget == nullptr)
		{
			return;
		}
	}

	FVector2D PixelPosition = InScreenPosition;
	FVector2D ViewportPosition = InScreenPosition;
	USlateBlueprintLibrary::AbsoluteToViewport(this, InScreenPosition, PixelPosition, ViewportPosition);

	ActiveDragPreviewWidget->ForceLayoutPrepass();
	const FVector2D PreviewSize = ActiveDragPreviewWidget->GetDesiredSize();
	const FVector2D PreviewPosition = ViewportPosition + (PreviewSize * InOperation->Offset);

	ActiveDragPreviewWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
	ActiveDragPreviewWidget->SetPositionInViewport(PreviewPosition, false);
}

void USMPlayerInventoryPanelWidget::ClearActiveDragPreview()
{
	ActiveDragPreviewOperation = nullptr;

	if (ActiveDragPreviewWidget != nullptr)
	{
		ActiveDragPreviewWidget->RemoveFromParent();
		ActiveDragPreviewWidget = nullptr;
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
	EnsureFloatingWidgetsCreated();

	if (ContextMenuWidget == nullptr)
	{
		return;
	}

	ContextMenuWidget->InitializeContextMenu(InItemInstanceId, InventoryComponent);
	if (ContextMenuWidget->HasAnyAvailableAction() == false)
	{
		CloseContextMenu();
		return;
	}

	ContextMenuWidget->SetVisibility(ESlateVisibility::Visible);
	ContextMenuScreenPosition = ClampFloatingWidgetToViewport(
		ContextMenuWidget,
		ResolveViewportPosition(InScreenPosition));
	ContextMenuWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
	ContextMenuWidget->SetPositionInViewport(ContextMenuScreenPosition, false);
	BP_OnContextMenuStateChanged();
}

void USMPlayerInventoryPanelWidget::CloseContextMenu()
{
	ContextMenuScreenPosition = FVector2D::ZeroVector;

	if (ContextMenuWidget != nullptr)
	{
		ContextMenuWidget->InitializeContextMenu(FGuid(), InventoryComponent);
		ContextMenuWidget->SetPositionInViewport(FVector2D::ZeroVector, false);
		ContextMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
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

	EnsureFloatingWidgetsCreated();

	const bool bHoveredItemChanged = HoveredItemInstanceId != InItemInstanceId;
	HoveredItemInstanceId = InItemInstanceId;

	if (ItemHoverInfoWidget != nullptr)
	{
		const FVector2D HoverInfoPosition = ResolveViewportPosition(InScreenPosition) + FVector2D(5.0f, 5.0f);

		if (ItemHoverInfoWidget->GetItemInstanceId() == InItemInstanceId && ItemHoverInfoWidget->IsShowingItemInfo())
		{
			ItemHoverInfoWidget->UpdateScreenPosition(HoverInfoPosition);
		}
		else
		{
			ItemHoverInfoWidget->ShowItemInfo(InItemInstanceId, HoverInfoPosition);
		}

		ItemHoverInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		ItemHoverInfoWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		ItemHoverInfoWidget->SetPositionInViewport(
			ClampFloatingWidgetToViewport(ItemHoverInfoWidget, HoverInfoPosition),
			false);
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
		ItemHoverInfoWidget->SetPositionInViewport(FVector2D::ZeroVector, false);
		ItemHoverInfoWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (bWasHovered)
	{
		BP_OnHoveredItemChanged();
	}
}

void USMPlayerInventoryPanelWidget::InitializeChildWidgets()
{
	EnsureFloatingWidgetsCreated();

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

	if (InventoryNotificationWidget != nullptr)
	{
		InventoryNotificationWidget->SetListenChannel(SMUITag::Event_Notification_Inventory);
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
