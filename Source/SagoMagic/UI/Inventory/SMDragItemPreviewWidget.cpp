#include "UI/Inventory/SMDragItemPreviewWidget.h"

USMDragItemPreviewWidget::USMDragItemPreviewWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , PreviewRotation(0)
	  , bCanPlaceOnCurrentCell(false)
{
}

void USMDragItemPreviewWidget::InitializePreview(const FGuid& InItemInstanceId, int32 InPreviewRotation)
{
	ItemInstanceId = InItemInstanceId;
	PreviewRotation = InPreviewRotation;
	bCanPlaceOnCurrentCell = false;

	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePreviewRotation(int32 InPreviewRotation)
{
	PreviewRotation = InPreviewRotation;

	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePlaceableState(bool bInCanPlaceOnCurrentCell)
{
	bCanPlaceOnCurrentCell = bInCanPlaceOnCurrentCell;

	BP_OnPreviewDataChanged();
}
