#include "UI/Inventory/SMDragItemPreviewWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"
#include "Inventory/Items/Fragments/SMGridShapeFragment.h"

USMDragItemPreviewWidget::USMDragItemPreviewWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , AccentColor(FLinearColor::White)
	  , DisplayItemType(ESMItemType::None)
	  , PreviewRotation(ESMGridRotation::Rot0)
	  , bCanPlaceOnCurrentCell(false)
	  , PreviewGrid(nullptr)
	  , PreviewCellSize(16.0f)
	  , PreviewCellPadding(1.0f)
{
}

void USMDragItemPreviewWidget::InitializePreview(const FGuid& InItemInstanceId, ESMGridRotation InPreviewRotation)
{
	ItemInstanceId = InItemInstanceId;
	InventoryComponent = nullptr;
	DisplayName = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	DisplayItemType = ESMItemType::None;
	ShapeMask = FSMGridMaskData();
	PreviewRotation = InPreviewRotation;
	bCanPlaceOnCurrentCell = false;

	RebuildPreviewGrid();
	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::InitializePreviewFromInventory(
	const FGuid& InItemInstanceId,
	ESMGridRotation InPreviewRotation,
	USMInventoryComponent* InInventoryComponent)
{
	ItemInstanceId = InItemInstanceId;
	InventoryComponent = InInventoryComponent;
	PreviewRotation = InPreviewRotation;
	bCanPlaceOnCurrentCell = false;

	SyncFromInventoryComponent();
	RebuildPreviewGrid();
	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePreviewRotation(ESMGridRotation InPreviewRotation)
{
	PreviewRotation = InPreviewRotation;

	RebuildPreviewGrid();
	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::UpdatePlaceableState(bool bInCanPlaceOnCurrentCell)
{
	bCanPlaceOnCurrentCell = bInCanPlaceOnCurrentCell;

	RebuildPreviewGrid();
	BP_OnPreviewDataChanged();
}

void USMDragItemPreviewWidget::SyncFromInventoryComponent()
{
	DisplayName = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	DisplayItemType = ESMItemType::None;
	ShapeMask = FSMGridMaskData();

	if (InventoryComponent == nullptr || ItemInstanceId.IsValid() == false)
	{
		return;
	}

	FSMItemInstanceData BaseItemData;

	FSMItemInstanceData ItemData;
	if (InventoryComponent->GetItemData(ItemInstanceId, ItemData))
	{
		BaseItemData = ItemData;
	}
	else
	{
		FSMSkillItemInstanceData SkillData;
		if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData) == false)
		{
			return;
		}

		BaseItemData = SkillData.BaseItem;
	}

	DisplayItemType = BaseItemData.ItemType;

	const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(BaseItemData);
	if (ItemDefinition == nullptr)
	{
		return;
	}

	DisplayName = FText::FromName(ItemDefinition->GetInternalName());

	const USMDisplayInfoFragment* DisplayInfoFragment = ItemDefinition->FindFragmentByClass<USMDisplayInfoFragment>();
	if (DisplayInfoFragment != nullptr)
	{
		if (DisplayInfoFragment->GetDisplayName().IsEmpty() == false)
		{
			DisplayName = DisplayInfoFragment->GetDisplayName();
		}

		AccentColor = DisplayInfoFragment->GetAccentColor();
	}

	const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
	if (GridShapeFragment != nullptr && GridShapeFragment->GetShapeMask().IsValidMaskData())
	{
		ShapeMask = GridShapeFragment->GetShapeMask();
	}
}

void USMDragItemPreviewWidget::RebuildPreviewGrid()
{
	if (PreviewGrid == nullptr || WidgetTree == nullptr)
	{
		return;
	}

	PreviewGrid->ClearChildren();
	PreviewGrid->SetSlotPadding(FMargin(PreviewCellPadding));

	const FLinearColor PreviewCellColor = bCanPlaceOnCurrentCell
		                                      ? AccentColor
		                                      : FLinearColor(0.85f, 0.25f, 0.25f, 0.9f);

	auto AddPreviewCell = [&](int32 InColumn, int32 InRow)
	{
		USizeBox* PreviewCellSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (PreviewCellSizeBox == nullptr)
		{
			return;
		}

		PreviewCellSizeBox->SetWidthOverride(PreviewCellSize);
		PreviewCellSizeBox->SetHeightOverride(PreviewCellSize);

		UBorder* PreviewCellBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (PreviewCellBorder == nullptr)
		{
			return;
		}

		PreviewCellBorder->SetBrushColor(PreviewCellColor);
		PreviewCellSizeBox->SetContent(PreviewCellBorder);

		if (UUniformGridSlot* PreviewCellSlot =
			PreviewGrid->AddChildToUniformGrid(PreviewCellSizeBox, InRow, InColumn))
		{
			PreviewCellSlot->SetHorizontalAlignment(HAlign_Fill);
			PreviewCellSlot->SetVerticalAlignment(VAlign_Fill);
		}
	};

	if (ShapeMask.IsValidMaskData() == false)
	{
		AddPreviewCell(0, 0);
		return;
	}

	bool bAddedAnyCell = false;

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

			AddPreviewCell(PreviewColumn, PreviewRow);
			bAddedAnyCell = true;
		}
	}

	if (bAddedAnyCell == false)
	{
		AddPreviewCell(0, 0);
	}
}

bool USMDragItemPreviewWidget::CalculatePreviewCellPosition(
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

	switch (PreviewRotation)
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
