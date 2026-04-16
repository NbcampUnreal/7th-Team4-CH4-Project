#include "UI/Inventory/SMInteractionWorldInfoWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

USMInteractionWorldInfoWidget::USMInteractionWorldInfoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , AccentColor(FLinearColor::White)
	  , ShapeRotation(ESMGridRotation::Rot0)
	  , bHasDisplayData(false)
	  , ShapePreviewCanvas(nullptr)
{
}

void USMInteractionWorldInfoWidget::ShowInteractionInfo(const FSMInteractionWorldInfoData& InDisplayData)
{
	DisplayName = InDisplayData.DisplayName;
	Description = InDisplayData.Description;
	SummaryText = InDisplayData.SummaryText;
	AccentColor = InDisplayData.AccentColor;
	ShapeMask = InDisplayData.ShapeMask;
	ShapeRotation = InDisplayData.ShapeRotation;
	bHasDisplayData = true;

	ApplyDisplayDataToWidget();
	RebuildShapePreview();
}

void USMInteractionWorldInfoWidget::HideInteractionInfo()
{
	DisplayName = FText::GetEmpty();
	Description = FText::GetEmpty();
	SummaryText = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	ShapeMask = FSMGridMaskData();
	ShapeRotation = ESMGridRotation::Rot0;
	bHasDisplayData = false;

	ApplyDisplayDataToWidget();
	RebuildShapePreview();
}

void USMInteractionWorldInfoWidget::ApplyDisplayDataToWidget()
{
	if (InfoRoot != nullptr)
	{
		InfoRoot->SetVisibility(bHasDisplayData ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}

	if (DisplayNameTextBlock != nullptr)
	{
		DisplayNameTextBlock->SetText(DisplayName);
	}

	if (DescriptionTextBlock != nullptr)
	{
		DescriptionTextBlock->SetText(Description);
	}

	if (SummaryTextBlock != nullptr)
	{
		SummaryTextBlock->SetText(SummaryText);
	}

	if (AccentColorBorder != nullptr)
	{
		AccentColorBorder->SetBrushColor(AccentColor);
	}
}

void USMInteractionWorldInfoWidget::RebuildShapePreview()
{
	if (ShapePreviewCanvas == nullptr || WidgetTree == nullptr)
	{
		return;
	}

	ShapePreviewCanvas->ClearChildren();

	if (bHasDisplayData == false)
	{
		return;
	}

	TArray<FIntPoint> OccupiedCells;
	BuildOccupiedCells(OccupiedCells);

	if (OccupiedCells.IsEmpty())
	{
		OccupiedCells.Add(FIntPoint::ZeroValue);
	}

	int32 MinX = 0;
	int32 MinY = 0;
	int32 MaxX = 0;
	int32 MaxY = 0;
	CalculateOccupiedCellBounds(OccupiedCells, MinX, MinY, MaxX, MaxY);

	const int32 PreviewWidth = FMath::Max(1, MaxX - MinX + 1);
	const int32 PreviewHeight = FMath::Max(1, MaxY - MinY + 1);
	const float PreviewCellSpacing = PreviewCellPadding * 2.0f;
	const float PreviewOuterPadding = PreviewCellPadding * 2.0f;
	const FVector2D AvailablePreviewAreaSize(
		FMath::Max(0.0f, PreviewAreaSize.X - (PreviewOuterPadding * 2.0f)),
		FMath::Max(0.0f, PreviewAreaSize.Y - (PreviewOuterPadding * 2.0f)));
	const float WidthCellSize = PreviewAreaSize.X / static_cast<float>(PreviewWidth);
	const float HeightCellSize = PreviewAreaSize.Y / static_cast<float>(PreviewHeight);
	const float BasePreviewCellSize = FMath::Clamp(
		FMath::Min(WidthCellSize, HeightCellSize),
		MinPreviewCellSize,
		MaxPreviewCellSize);
	const float MaxContentWidth =
		(AvailablePreviewAreaSize.X - ((PreviewWidth - 1) * PreviewCellSpacing)) / static_cast<float>(PreviewWidth);
	const float MaxContentHeight =
		(AvailablePreviewAreaSize.Y - ((PreviewHeight - 1) * PreviewCellSpacing)) / static_cast<float>(PreviewHeight);
	const float PreviewCellContentSize = FMath::Clamp(
		BasePreviewCellSize,
		MinPreviewCellSize,
		FMath::Max(0.0f, FMath::Min(FMath::Min(MaxContentWidth, MaxContentHeight), MaxPreviewCellSize)));
	const float OccupiedPreviewWidth =
		(PreviewWidth * PreviewCellContentSize) + ((PreviewWidth - 1) * PreviewCellSpacing);
	const float OccupiedPreviewHeight =
		(PreviewHeight * PreviewCellContentSize) + ((PreviewHeight - 1) * PreviewCellSpacing);
	const FVector2D PreviewOffset(
		(PreviewAreaSize.X - OccupiedPreviewWidth) * 0.5f,
		(PreviewAreaSize.Y - OccupiedPreviewHeight) * 0.5f);

	for (const FIntPoint& OccupiedCell : OccupiedCells)
	{
		USizeBox* PreviewCellSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (PreviewCellSizeBox == nullptr)
		{
			continue;
		}

		PreviewCellSizeBox->SetVisibility(ESlateVisibility::HitTestInvisible);
		PreviewCellSizeBox->SetWidthOverride(PreviewCellContentSize);
		PreviewCellSizeBox->SetHeightOverride(PreviewCellContentSize);

		UBorder* PreviewCellBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (PreviewCellBorder == nullptr)
		{
			continue;
		}

		PreviewCellBorder->SetVisibility(ESlateVisibility::HitTestInvisible);
		PreviewCellBorder->SetBrushColor(AccentColor);
		PreviewCellSizeBox->SetContent(PreviewCellBorder);

		if (UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(ShapePreviewCanvas->AddChild(PreviewCellSizeBox)))
		{
			PreviewCanvasSlot->SetAutoSize(false);
			PreviewCanvasSlot->SetSize(FVector2D(PreviewCellContentSize, PreviewCellContentSize));
			PreviewCanvasSlot->SetPosition(FVector2D(
				PreviewOffset.X + ((OccupiedCell.X - MinX) * (PreviewCellContentSize + PreviewCellSpacing)),
				PreviewOffset.Y + ((OccupiedCell.Y - MinY) * (PreviewCellContentSize + PreviewCellSpacing))));
		}
	}
}

bool USMInteractionWorldInfoWidget::CalculatePreviewCellPosition(
	int32 InLocalX,
	int32 InLocalY,
	int32& OutColumn,
	int32& OutRow) const
{
	OutColumn = 0;
	OutRow = 0;

	if (ShapeMask.IsValidMaskData() == false)
	{
		return false;
	}

	switch (ShapeRotation)
	{
	case ESMGridRotation::Rot0:
		OutColumn = InLocalX;
		OutRow = InLocalY;
		return true;

	case ESMGridRotation::Rot90:
		OutColumn = ShapeMask.Height - 1 - InLocalY;
		OutRow = InLocalX;
		return true;

	case ESMGridRotation::Rot180:
		OutColumn = ShapeMask.Width - 1 - InLocalX;
		OutRow = ShapeMask.Height - 1 - InLocalY;
		return true;

	case ESMGridRotation::Rot270:
		OutColumn = InLocalY;
		OutRow = ShapeMask.Width - 1 - InLocalX;
		return true;

	default:
		return false;
	}
}

void USMInteractionWorldInfoWidget::BuildOccupiedCells(TArray<FIntPoint>& OutOccupiedCells) const
{
	OutOccupiedCells.Reset();

	if (ShapeMask.IsValidMaskData() == false)
	{
		return;
	}

	for (int32 LocalY = 0; LocalY < ShapeMask.Height; ++LocalY)
	{
		for (int32 LocalX = 0; LocalX < ShapeMask.Width; ++LocalX)
		{
			const int32 MaskIndex = (LocalY * ShapeMask.Width) + LocalX;
			if (ShapeMask.BitMask.IsValidIndex(MaskIndex) == false || ShapeMask.BitMask[MaskIndex] != TEXT('1'))
			{
				continue;
			}

			int32 PreviewColumn = 0;
			int32 PreviewRow = 0;
			if (CalculatePreviewCellPosition(LocalX, LocalY, PreviewColumn, PreviewRow) == false)
			{
				continue;
			}

			OutOccupiedCells.Add(FIntPoint(PreviewColumn, PreviewRow));
		}
	}
}

bool USMInteractionWorldInfoWidget::CalculateOccupiedCellBounds(
	const TArray<FIntPoint>& InOccupiedCells,
	int32& OutMinX,
	int32& OutMinY,
	int32& OutMaxX,
	int32& OutMaxY) const
{
	if (InOccupiedCells.IsEmpty())
	{
		return false;
	}

	OutMinX = InOccupiedCells[0].X;
	OutMinY = InOccupiedCells[0].Y;
	OutMaxX = InOccupiedCells[0].X;
	OutMaxY = InOccupiedCells[0].Y;

	for (int32 CellIndex = 1; CellIndex < InOccupiedCells.Num(); ++CellIndex)
	{
		const FIntPoint& OccupiedCell = InOccupiedCells[CellIndex];
		OutMinX = FMath::Min(OutMinX, OccupiedCell.X);
		OutMinY = FMath::Min(OutMinY, OccupiedCell.Y);
		OutMaxX = FMath::Max(OutMaxX, OccupiedCell.X);
		OutMaxY = FMath::Max(OutMaxY, OccupiedCell.Y);
	}

	return true;
}
