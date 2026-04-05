#include "UI/Inventory/SMInventoryCellWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMInventoryGridWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryCellWidget::USMInventoryCellWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , GridX(0)
	  , GridY(0)
	  , bCellEnabled(false)
	  , bHoveredCell(false)
	  , bPlaceableHighlighted(false)
	  , bBlockedHighlighted(false)
{
}

void USMInventoryCellWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (OwnerItemInstanceId.IsValid() == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->ShowHoveredItemInfo(OwnerItemInstanceId, InMouseEvent.GetScreenSpacePosition());
	}
}

FReply USMInventoryCellWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (OwnerItemInstanceId.IsValid() == false)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->ShowHoveredItemInfo(OwnerItemInstanceId, InMouseEvent.GetScreenSpacePosition());
	}

	return FReply::Handled();
}

void USMInventoryCellWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (OwnerItemInstanceId.IsValid() == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (OwningPanel->GetHoveredItemInstanceId() == OwnerItemInstanceId)
		{
			OwningPanel->HideHoveredItemInfo();
		}
	}
}

FReply USMInventoryCellWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (OwnerItemInstanceId.IsValid() == false)
			{
				OwningPanel->CloseContextMenu();
				return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
			}

			OwningPanel->ShowHoveredItemInfo(OwnerItemInstanceId, InMouseEvent.GetScreenSpacePosition());
			OwningPanel->OpenContextMenuForItem(OwnerItemInstanceId, InMouseEvent.GetScreenSpacePosition());
			return FReply::Handled();
		}

		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OwningPanel->CloseContextMenu();
		}
	}

	if (OwnerItemInstanceId.IsValid() == false)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void USMInventoryCellWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                                  UDragDropOperation*& OutOperation)
{
	OutOperation = nullptr;

	if (OwnerItemInstanceId.IsValid() == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->HideHoveredItemInfo();
		OwningPanel->CloseContextMenu();
	}

	if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
	{
		OutOperation = OwningGrid->CreateDragDropOperationForItem(OwnerItemInstanceId);
	}
}

void USMInventoryCellWidget::InitializeCellWidget(int32 InGridX, int32 InGridY, bool bInCellEnabled)
{
	GridX = InGridX;
	GridY = InGridY;
	bCellEnabled = bInCellEnabled;
	bHoveredCell = false;
	bPlaceableHighlighted = false;
	bBlockedHighlighted = false;
	OwnerItemInstanceId.Invalidate();

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::UpdateCellState(
	bool bInCellEnabled,
	bool bInHoveredCell,
	bool bInPlaceableHighlighted,
	bool bInBlockedHighlighted)
{
	bCellEnabled = bInCellEnabled;
	bHoveredCell = bInHoveredCell;
	bPlaceableHighlighted = bInPlaceableHighlighted;
	bBlockedHighlighted = bInBlockedHighlighted;

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::ClearHighlightState()
{
	bHoveredCell = false;
	bPlaceableHighlighted = false;
	bBlockedHighlighted = false;

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::UpdateOccupiedItem(const FGuid& InOwnerItemInstanceId)
{
	if (OwnerItemInstanceId == InOwnerItemInstanceId)
	{
		return;
	}

	OwnerItemInstanceId = InOwnerItemInstanceId;
	BP_OnCellStateChanged();
}
