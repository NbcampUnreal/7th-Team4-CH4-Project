#include "UI/Inventory/SMDragItemPreviewWidget.h"

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
