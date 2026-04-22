#include "UI/Inventory/SMInventoryCellVisualWidget.h"

#include "Components/Border.h"

USMInventoryCellVisualWidget::USMInventoryCellVisualWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , Outline(nullptr)
	  , OccupiedFill(nullptr)
	  , bShowOutline(true)
	  , bShowOccupiedFill(false)
	  , OccupiedAccentColor(FLinearColor::White)
{
}

void USMInventoryCellVisualWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	ApplyVisualState();
}

void USMInventoryCellVisualWidget::ResetVisualState()
{
	bShowOutline = true;
	bShowOccupiedFill = false;
	OccupiedAccentColor = FLinearColor::White;

	ApplyVisualState();
}

void USMInventoryCellVisualWidget::UpdateVisualState(
	bool bInShowOutline,
	bool bInShowOccupiedFill,
	FLinearColor InOccupiedAccentColor)
{
	if (bShowOutline == bInShowOutline &&
		bShowOccupiedFill == bInShowOccupiedFill &&
		OccupiedAccentColor.Equals(InOccupiedAccentColor))
	{
		return;
	}

	bShowOutline = bInShowOutline;
	bShowOccupiedFill = bInShowOccupiedFill;
	OccupiedAccentColor = InOccupiedAccentColor;

	ApplyVisualState();
}

void USMInventoryCellVisualWidget::SetOutlineVisible(bool bInShowOutline)
{
	if (bShowOutline == bInShowOutline)
	{
		return;
	}

	bShowOutline = bInShowOutline;
	ApplyVisualState();
}

void USMInventoryCellVisualWidget::SetOccupiedFillVisible(bool bInShowOccupiedFill)
{
	if (bShowOccupiedFill == bInShowOccupiedFill)
	{
		return;
	}

	bShowOccupiedFill = bInShowOccupiedFill;
	ApplyVisualState();
}

void USMInventoryCellVisualWidget::SetOccupiedAccentColor(FLinearColor InOccupiedAccentColor)
{
	if (OccupiedAccentColor.Equals(InOccupiedAccentColor))
	{
		return;
	}

	OccupiedAccentColor = InOccupiedAccentColor;
	ApplyVisualState();
}

void USMInventoryCellVisualWidget::ApplyVisualState()
{
	if (Outline != nullptr)
	{
		Outline->SetVisibility(bShowOutline ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (OccupiedFill != nullptr)
	{
		OccupiedFill->SetVisibility(bShowOccupiedFill ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		OccupiedFill->SetBrushColor(OccupiedAccentColor);
	}
}
