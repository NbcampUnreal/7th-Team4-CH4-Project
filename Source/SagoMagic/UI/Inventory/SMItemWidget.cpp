#include "UI/Inventory/SMItemWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMItemWidget::USMItemWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , GridX(0)
	  , GridY(0)
	  , DisplayRotation(ESMGridRotation::Rot0)
	  , bDraggable(true)
	  , InventoryComponent(nullptr)
{
}

void USMItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply USMItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ItemInstanceId.IsValid() == false || InventoryComponent == nullptr)
		{
			return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
		}

		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->SetHoveredItem(ItemInstanceId);
			OwningPanel->OpenContextMenuForItem(ItemInstanceId, InMouseEvent.GetScreenSpacePosition());
			return FReply::Handled();
		}

		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->CloseContextMenu();
		}
	}

	if (CanStartDrag() == false)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void USMItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (ItemInstanceId.IsValid() == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->SetHoveredItem(ItemInstanceId);
	}
}

void USMItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (OwningPanel->GetHoveredItemInstanceId() == ItemInstanceId)
		{
			OwningPanel->ClearHoveredItem();
		}
	}
}

void USMItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                         UDragDropOperation*& OutOperation)
{
	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (OwningPanel->GetHoveredItemInstanceId() == ItemInstanceId)
		{
			OwningPanel->ClearHoveredItem();
		}

		OwningPanel->CloseContextMenu();
	}

	OutOperation = CreateDragDropOperation();

	if (USMInventoryDragDropOperation* InventoryOperation = Cast<USMInventoryDragDropOperation>(OutOperation))
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->BeginActiveDragPreview(InventoryOperation, InMouseEvent.GetScreenSpacePosition());
		}
	}
}

void USMItemWidget::InitializeItemWidget(
	const FGuid& InItemInstanceId,
	const FGuid& InOwningContainerId,
	int32 InGridX,
	int32 InGridY,
	ESMGridRotation InDisplayRotation,
	USMInventoryComponent* InInventoryComponent)
{
	ItemInstanceId = InItemInstanceId;
	OwningContainerId = InOwningContainerId;
	GridX = InGridX;
	GridY = InGridY;
	DisplayRotation = InDisplayRotation;
	InventoryComponent = InInventoryComponent;

	RefreshItemWidget();
}

void USMItemWidget::RefreshItemWidget()
{
	UpdateDisplayFromInventory();
	BP_OnItemWidgetUpdated();
}

bool USMItemWidget::CanStartDrag() const
{
	return bDraggable && ItemInstanceId.IsValid() && InventoryComponent != nullptr;
}

USMInventoryDragDropOperation* USMItemWidget::CreateDragDropOperation()
{
	USMInventoryDragDropOperation* NewOperation = NewObject<USMInventoryDragDropOperation>(this);
	if (NewOperation == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = CreateDragPreviewWidget();

	NewOperation->InitializeOperation(
		ItemInstanceId,
		OwningContainerId,
		GridX,
		GridY,
		DisplayRotation,
		0,
		0,
		1,
		1,
		FVector2D(0.5f, 0.5f),
		PreviewWidget);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		NewOperation->SetOwningInventoryPanel(OwningPanel);
	}

	return NewOperation;
}

USMDragItemPreviewWidget* USMItemWidget::CreateDragPreviewWidget()
{
	if (DragPreviewWidgetClass == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = CreateWidget<USMDragItemPreviewWidget>(this, DragPreviewWidgetClass);
	if (PreviewWidget == nullptr)
	{
		return nullptr;
	}

	PreviewWidget->InitializePreviewFromInventory(ItemInstanceId, DisplayRotation, InventoryComponent);
	return PreviewWidget;
}

void USMItemWidget::UpdateDisplayFromInventory()
{
	if (InventoryComponent == nullptr)
	{
		bDraggable = false;
		return;
	}

	FSMItemInstanceData ItemData;
	if (InventoryComponent->GetItemData(ItemInstanceId, ItemData))
	{
		OwningContainerId = ItemData.ParentContainerId;
		GridX = ItemData.GridX;
		GridY = ItemData.GridY;
		DisplayRotation = ItemData.Rotation;
		bDraggable = ItemData.bLocked == false;
		return;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData))
	{
		OwningContainerId = SkillData.BaseItem.ParentContainerId;
		GridX = SkillData.BaseItem.GridX;
		GridY = SkillData.BaseItem.GridY;
		DisplayRotation = SkillData.BaseItem.Rotation;
		bDraggable = SkillData.BaseItem.bLocked == false;
		return;
	}

	bDraggable = false;
}
