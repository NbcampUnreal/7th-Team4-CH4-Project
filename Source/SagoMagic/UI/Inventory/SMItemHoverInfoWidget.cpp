#include "UI/Inventory/SMItemHoverInfoWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"

USMItemHoverInfoWidget::USMItemHoverInfoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , AccentColor(FLinearColor::White)
	  , DisplayItemType(ESMItemType::None)
	  , ScreenPosition(FVector2D::ZeroVector)
	  , bIsShowingItemInfo(false)
{
}

void USMItemHoverInfoWidget::InitializeHoverInfoWidget(USMInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;
	HideItemInfo();
}

void USMItemHoverInfoWidget::ShowItemInfo(const FGuid& InItemInstanceId, FVector2D InScreenPosition)
{
	ItemInstanceId = InItemInstanceId;
	ScreenPosition = InScreenPosition;
	bIsShowingItemInfo = ItemInstanceId.IsValid();

	SyncFromInventoryComponent();
	BP_OnHoverInfoUpdated();
}

void USMItemHoverInfoWidget::HideItemInfo()
{
	ItemInstanceId.Invalidate();
	DisplayName = FText::GetEmpty();
	Description = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	DisplayItemType = ESMItemType::None;
	ScreenPosition = FVector2D::ZeroVector;
	bIsShowingItemInfo = false;

	BP_OnHoverInfoUpdated();
}

void USMItemHoverInfoWidget::RefreshItemInfo()
{
	if (bIsShowingItemInfo == false || ItemInstanceId.IsValid() == false)
	{
		HideItemInfo();
		return;
	}

	SyncFromInventoryComponent();
	BP_OnHoverInfoUpdated();
}

void USMItemHoverInfoWidget::SyncFromInventoryComponent()
{
	DisplayName = FText::GetEmpty();
	Description = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	DisplayItemType = ESMItemType::None;

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
	if (DisplayInfoFragment == nullptr)
	{
		return;
	}

	if (DisplayInfoFragment->GetDisplayName().IsEmpty() == false)
	{
		DisplayName = DisplayInfoFragment->GetDisplayName();
	}

	Description = DisplayInfoFragment->GetDescription();
	AccentColor = DisplayInfoFragment->GetAccentColor();
}
