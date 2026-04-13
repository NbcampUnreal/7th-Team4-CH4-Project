#include "UI/Inventory/SMInventoryCellWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"
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
	  , bOccupiedCell(false)
	  , OccupiedAccentColor(FLinearColor::White)
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

	if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
	{
		OwningGrid->SetHoveredItemInstanceId(OwnerItemInstanceId);
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

	if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
	{
		OwningGrid->SetHoveredItemInstanceId(OwnerItemInstanceId);
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

	if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
	{
		OwningGrid->SetHoveredItemInstanceId(FGuid());
	}
}

FReply USMInventoryCellWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
			{
				FSMGridContainerState ContainerData;
				if (OwningGrid->GetInventoryComponent() != nullptr &&
					OwningGrid->GetInventoryComponent()->GetContainerData(OwningGrid->GetContainerId(), ContainerData) &&
					ContainerData.ContainerType == ESMContainerType::SkillInternal)
				{
					OwningPanel->CloseContextMenu();
					return FReply::Handled();
				}
			}

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
		OwningGrid->SetHoveredItemInstanceId(FGuid());
		
		const FVector2D LocalMousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		const FVector2D LocalSize = InGeometry.GetLocalSize();

		FVector2D PivotCellFraction(0.5f, 0.5f);
		if (LocalSize.X > 0.0f && LocalSize.Y > 0.0f)
		{
			PivotCellFraction.X = FMath::Clamp(LocalMousePosition.X / LocalSize.X, 0.0f, 1.0f);
			PivotCellFraction.Y = FMath::Clamp(LocalMousePosition.Y / LocalSize.Y, 0.0f, 1.0f);
		}

		OutOperation = OwningGrid->CreateDragDropOperationForItem(
			OwnerItemInstanceId,
			GridX,
			GridY,
			PivotCellFraction);

		if (USMInventoryDragDropOperation* InventoryOperation = Cast<USMInventoryDragDropOperation>(OutOperation))
		{
			if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
			{
				OwningPanel->BeginActiveDragPreview(InventoryOperation, InMouseEvent.GetScreenSpacePosition());
			}
		}
	}
}

void USMInventoryCellWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->ClearActiveDragState();
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
	bOccupiedCell = false;
	OccupiedAccentColor = FLinearColor::White;

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::UpdateCellState(
	bool bInCellEnabled,
	bool bInHoveredCell,
	bool bInPlaceableHighlighted,
	bool bInBlockedHighlighted)
{
	const bool bShouldDisplayOccupiedState =
		OwnerItemInstanceId.IsValid() &&
		bInPlaceableHighlighted == false &&
		bInBlockedHighlighted == false;

	if (bCellEnabled == bInCellEnabled &&
		bHoveredCell == bInHoveredCell &&
		bPlaceableHighlighted == bInPlaceableHighlighted &&
		bBlockedHighlighted == bInBlockedHighlighted &&
		bOccupiedCell == bShouldDisplayOccupiedState)
	{
		return;
	}

	bCellEnabled = bInCellEnabled;
	bHoveredCell = bInHoveredCell;
	bPlaceableHighlighted = bInPlaceableHighlighted;
	bBlockedHighlighted = bInBlockedHighlighted;
	bOccupiedCell = bShouldDisplayOccupiedState;

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::ClearHighlightState()
{
	bHoveredCell = false;
	bPlaceableHighlighted = false;
	bBlockedHighlighted = false;

	BP_OnCellStateChanged();
}

void USMInventoryCellWidget::UpdateOccupiedItem(const FGuid& InOwnerItemInstanceId, const FLinearColor& InOccupiedAccentColor)
{
	const bool bInOccupiedCell = InOwnerItemInstanceId.IsValid();
	if (OwnerItemInstanceId == InOwnerItemInstanceId &&
		bOccupiedCell == bInOccupiedCell &&
		OccupiedAccentColor.Equals(InOccupiedAccentColor))
	{
		return;
	}

	OwnerItemInstanceId = InOwnerItemInstanceId;
	bOccupiedCell = bInOccupiedCell;
	OccupiedAccentColor = bOccupiedCell ? InOccupiedAccentColor : FLinearColor::White;
	BP_OnCellStateChanged();
}
