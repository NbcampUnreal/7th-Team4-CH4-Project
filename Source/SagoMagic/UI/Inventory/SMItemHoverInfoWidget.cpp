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
	  , TypeDescription(FText::GetEmpty())
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

void USMItemHoverInfoWidget::UpdateScreenPosition(FVector2D InScreenPosition)
{
	if (bIsShowingItemInfo == false)
	{
		return;
	}

	ScreenPosition = InScreenPosition;
	BP_OnHoverInfoUpdated();
}

void USMItemHoverInfoWidget::HideItemInfo()
{
	ItemInstanceId.Invalidate();
	DisplayName = FText::GetEmpty();
	Description = FText::GetEmpty();
	AccentColor = FLinearColor::White;
	DisplayItemType = ESMItemType::None;
	TypeDescription = FText::GetEmpty();
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
	TypeDescription = FText::GetEmpty();

	if (InventoryComponent == nullptr || ItemInstanceId.IsValid() == false)
	{
		return;
	}

	FSMItemInstanceData BaseItemData;
	FSMSkillItemInstanceData SkillData;
	bool bIsSkillItem = false;

	FSMItemInstanceData ItemData;
	if (InventoryComponent->GetItemData(ItemInstanceId, ItemData))
	{
		BaseItemData = ItemData;
	}
	else
	{
		if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData) == false)
		{
			return;
		}

		BaseItemData = SkillData.BaseItem;
		bIsSkillItem = true;
	}

	DisplayItemType = BaseItemData.ItemType;

	switch (DisplayItemType)
	{
	case ESMItemType::Gem:
		TypeDescription = NSLOCTEXT("SMItemHoverInfoWidget", "TypeDescription_Gem", "젬");
		break;

	case ESMItemType::Skill:
		if (bIsSkillItem)
		{
			const int32 CurrentLevel = FMath::Max(1, SkillData.GetCachedSummary().GetCurrentLevel());
			const bool bHasEmbeddedGem = SkillData.GetCachedSummary().EmbeddedGemIds.Num() > 0;
			TypeDescription = bHasEmbeddedGem
				                  ? FText::Format(
					                  NSLOCTEXT("SMItemHoverInfoWidget", "TypeDescription_SkillWithGem", "스킬 (레벨 {0}) / 젬 장착됨"),
					                  FText::AsNumber(CurrentLevel))
				                  : FText::Format(
					                  NSLOCTEXT("SMItemHoverInfoWidget", "TypeDescription_Skill", "스킬 (레벨 {0})"),
					                  FText::AsNumber(CurrentLevel));
		}
		else
		{
			TypeDescription = NSLOCTEXT("SMItemHoverInfoWidget", "TypeDescription_SkillFallback", "스킬");
		}
		break;

	default:
		TypeDescription = NSLOCTEXT("SMItemHoverInfoWidget", "TypeDescription_None", "None");
		break;
	}

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
