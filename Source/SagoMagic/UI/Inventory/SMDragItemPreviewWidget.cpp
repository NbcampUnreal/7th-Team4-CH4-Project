#include "UI/Inventory/SMDragItemPreviewWidget.h"

USMDragItemPreviewWidget::USMDragItemPreviewWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , PreviewRotation(ESMGridRotation::Rot0)
	  , bCanPlaceOnCurrentCell(false)
{
}

void USMDragItemPreviewWidget::InitializePreview(const FGuid& InItemInstanceId, ESMGridRotation InPreviewRotation)
{
	ItemInstanceId = InItemInstanceId;
	PreviewRotation = InPreviewRotation;
	bCanPlaceOnCurrentCell = false;

	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePreviewRotation(ESMGridRotation InPreviewRotation)
{
	PreviewRotation = InPreviewRotation;

	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePlaceableState(bool bInCanPlaceOnCurrentCell)
{
	bCanPlaceOnCurrentCell = bInCanPlaceOnCurrentCell;

	BP_OnPreviewDataChanged();
}
