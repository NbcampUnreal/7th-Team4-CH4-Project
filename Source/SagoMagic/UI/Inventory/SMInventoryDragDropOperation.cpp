#include "UI/Inventory/SMInventoryDragDropOperation.h"

#include "UI/Inventory/SMDragItemPreviewWidget.h"

USMInventoryDragDropOperation::USMInventoryDragDropOperation()
	: SourceGridX(0)
	  , SourceGridY(0)
	  , StartRotation(0)
	  , CurrentRotation(0)
	  , DragPreviewWidget(nullptr)
{
}

void USMInventoryDragDropOperation::InitializeOperation(
	const FGuid& InItemInstanceId,
	const FGuid& InSourceContainerId,
	int32 InSourceGridX,
	int32 InSourceGridY,
	int32 InStartRotation,
	USMDragItemPreviewWidget* InDragPreviewWidget)
{
	ItemInstanceId = InItemInstanceId;
	SourceContainerId = InSourceContainerId;
	SourceGridX = InSourceGridX;
	SourceGridY = InSourceGridY;
	StartRotation = InStartRotation;
	CurrentRotation = InStartRotation;
	DragPreviewWidget = InDragPreviewWidget;
}

void USMInventoryDragDropOperation::UpdateCurrentRotation(int32 InCurrentRotation)
{
	CurrentRotation = InCurrentRotation;
}

bool USMInventoryDragDropOperation::HasValidItemInstanceId() const
{
	return ItemInstanceId.IsValid();
}
