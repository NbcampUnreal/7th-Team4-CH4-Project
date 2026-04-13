#include "UI/Inventory/SMInventoryDragDropOperation.h"

#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryDragDropOperation::USMInventoryDragDropOperation()
	: SourceGridX(0)
	  , SourceGridY(0)
	  , StartRotation(ESMGridRotation::Rot0)
	  , CurrentRotation(ESMGridRotation::Rot0)
	  , PivotShapeLocalX(0)
	  , PivotShapeLocalY(0)
	  , ShapeWidth(1)
	  , ShapeHeight(1)
	  , PivotCellFraction(FVector2D(0.5f, 0.5f))
	  , PointerFromCenter(FVector2D::ZeroVector)
	  , DragPreviewWidget(nullptr)
	  , OwningInventoryPanel(nullptr)
{
}

void USMInventoryDragDropOperation::InitializeOperation(
	const FGuid& InItemInstanceId,
	const FGuid& InSourceContainerId,
	int32 InSourceGridX,
	int32 InSourceGridY,
	ESMGridRotation InStartRotation,
	int32 InPivotShapeLocalX,
	int32 InPivotShapeLocalY,
	int32 InShapeWidth,
	int32 InShapeHeight,
	FVector2D InPivotCellFraction,
	USMDragItemPreviewWidget* InDragPreviewWidget)
{
	ItemInstanceId = InItemInstanceId;
	SourceContainerId = InSourceContainerId;
	SourceGridX = InSourceGridX;
	SourceGridY = InSourceGridY;
	StartRotation = InStartRotation;
	CurrentRotation = InStartRotation;
	PivotShapeLocalX = InPivotShapeLocalX;
	PivotShapeLocalY = InPivotShapeLocalY;
	ShapeWidth = FMath::Max(1, InShapeWidth);
	ShapeHeight = FMath::Max(1, InShapeHeight);
	PivotCellFraction.X = FMath::Clamp(InPivotCellFraction.X, 0.0f, 1.0f);
	PivotCellFraction.Y = FMath::Clamp(InPivotCellFraction.Y, 0.0f, 1.0f);
	DragPreviewWidget = InDragPreviewWidget;

	FVector2D StartPointerLocal = FVector2D::ZeroVector;
	if (CalculatePointerLocalFromPivotData(StartRotation, StartPointerLocal))
	{
		const int32 StartWidth = (StartRotation == ESMGridRotation::Rot90 || StartRotation == ESMGridRotation::Rot270)
			                         ? ShapeHeight
			                         : ShapeWidth;
		const int32 StartHeight = (StartRotation == ESMGridRotation::Rot90 || StartRotation == ESMGridRotation::Rot270)
			                          ? ShapeWidth
			                          : ShapeHeight;
		const FVector2D StartCenter(
			static_cast<float>(StartWidth) * 0.5f,
			static_cast<float>(StartHeight) * 0.5f);
		PointerFromCenter = StartPointerLocal - StartCenter;
	}
	else
	{
		PointerFromCenter = FVector2D::ZeroVector;
	}

	UpdateDragVisualOffset();
}

void USMInventoryDragDropOperation::UpdateCurrentRotation(ESMGridRotation InCurrentRotation)
{
	CurrentRotation = InCurrentRotation;
	UpdateDragVisualOffset();
}

void USMInventoryDragDropOperation::Drop_Implementation(const FPointerEvent& PointerEvent)
{
	if (OwningInventoryPanel != nullptr)
	{
		OwningInventoryPanel->ClearActiveDragPreview();
	}

	Super::Drop_Implementation(PointerEvent);
}

void USMInventoryDragDropOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
	if (OwningInventoryPanel != nullptr)
	{
		OwningInventoryPanel->ClearActiveDragPreview();
	}

	Super::DragCancelled_Implementation(PointerEvent);
}

void USMInventoryDragDropOperation::Dragged_Implementation(const FPointerEvent& PointerEvent)
{
	if (OwningInventoryPanel != nullptr)
	{
		OwningInventoryPanel->UpdateActiveDragPreviewPosition(this, PointerEvent.GetScreenSpacePosition());
	}

	Super::Dragged_Implementation(PointerEvent);
}

bool USMInventoryDragDropOperation::HasValidItemInstanceId() const
{
	return ItemInstanceId.IsValid();
}

bool USMInventoryDragDropOperation::CalculateCurrentPivotOffset(int32& OutPivotOffsetX, int32& OutPivotOffsetY) const
{
	OutPivotOffsetX = 0;
	OutPivotOffsetY = 0;

	int32 CurrentWidth = 0;
	int32 CurrentHeight = 0;
	if (GetCurrentDimensions(CurrentWidth, CurrentHeight) == false)
	{
		return false;
	}

	const FVector2D CurrentCenter(
		static_cast<float>(CurrentWidth) * 0.5f,
		static_cast<float>(CurrentHeight) * 0.5f);
	const FVector2D CurrentPointerLocal = CurrentCenter + PointerFromCenter;

	OutPivotOffsetX = FMath::FloorToInt(CurrentPointerLocal.X);
	OutPivotOffsetY = FMath::FloorToInt(CurrentPointerLocal.Y);
	return true;
}

bool USMInventoryDragDropOperation::GetCurrentDimensions(int32& OutWidth, int32& OutHeight) const
{
	OutWidth = 0;
	OutHeight = 0;

	if (ShapeWidth <= 0 || ShapeHeight <= 0)
	{
		return false;
	}

	const bool bSwapDimensions = CurrentRotation == ESMGridRotation::Rot90 || CurrentRotation == ESMGridRotation::Rot270;
	OutWidth = bSwapDimensions ? ShapeHeight : ShapeWidth;
	OutHeight = bSwapDimensions ? ShapeWidth : ShapeHeight;
	return true;
}

bool USMInventoryDragDropOperation::CalculatePointerLocalFromPivotData(
	ESMGridRotation InRotation,
	FVector2D& OutPointerLocal) const
{
	OutPointerLocal = FVector2D::ZeroVector;

	if (ShapeWidth <= 0 || ShapeHeight <= 0)
	{
		return false;
	}

	int32 PivotOffsetX = 0;
	int32 PivotOffsetY = 0;
	FVector2D RotatedCellFraction = PivotCellFraction;

	switch (InRotation)
	{
	case ESMGridRotation::Rot0:
		PivotOffsetX = PivotShapeLocalX;
		PivotOffsetY = PivotShapeLocalY;
		break;

	case ESMGridRotation::Rot90:
		PivotOffsetX = ShapeHeight - 1 - PivotShapeLocalY;
		PivotOffsetY = PivotShapeLocalX;
		RotatedCellFraction = FVector2D(1.0f - PivotCellFraction.Y, PivotCellFraction.X);
		break;

	case ESMGridRotation::Rot180:
		PivotOffsetX = ShapeWidth - 1 - PivotShapeLocalX;
		PivotOffsetY = ShapeHeight - 1 - PivotShapeLocalY;
		RotatedCellFraction = FVector2D(1.0f - PivotCellFraction.X, 1.0f - PivotCellFraction.Y);
		break;

	case ESMGridRotation::Rot270:
		PivotOffsetX = PivotShapeLocalY;
		PivotOffsetY = ShapeWidth - 1 - PivotShapeLocalX;
		RotatedCellFraction = FVector2D(PivotCellFraction.Y, 1.0f - PivotCellFraction.X);
		break;

	default:
		return false;
	}

	OutPointerLocal.X = static_cast<float>(PivotOffsetX) + RotatedCellFraction.X;
	OutPointerLocal.Y = static_cast<float>(PivotOffsetY) + RotatedCellFraction.Y;
	return true;
}

void USMInventoryDragDropOperation::UpdateDragVisualOffset()
{
	Pivot = EDragPivot::TopLeft;

	int32 CurrentWidth = 0;
	int32 CurrentHeight = 0;
	if (GetCurrentDimensions(CurrentWidth, CurrentHeight) == false)
	{
		Offset = FVector2D::ZeroVector;
		return;
	}

	const FVector2D CurrentCenter(
		static_cast<float>(CurrentWidth) * 0.5f,
		static_cast<float>(CurrentHeight) * 0.5f);
	const FVector2D CurrentPointerLocal = CurrentCenter + PointerFromCenter;
	const float PointerFractionX = CurrentPointerLocal.X / static_cast<float>(FMath::Max(1, CurrentWidth));
	const float PointerFractionY = CurrentPointerLocal.Y / static_cast<float>(FMath::Max(1, CurrentHeight));

	Offset = FVector2D(-PointerFractionX, -PointerFractionY);
}
