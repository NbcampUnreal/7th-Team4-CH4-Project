#include "UI/Inventory/SMInventoryCellWidget.h"

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

void USMInventoryCellWidget::InitializeCellWidget(int32 InGridX, int32 InGridY, bool bInCellEnabled)
{
	GridX = InGridX;
	GridY = InGridY;
	bCellEnabled = bInCellEnabled;
	bHoveredCell = false;
	bPlaceableHighlighted = false;
	bBlockedHighlighted = false;

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
