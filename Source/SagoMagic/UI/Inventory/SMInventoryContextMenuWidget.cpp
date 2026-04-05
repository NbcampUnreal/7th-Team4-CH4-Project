#include "UI/Inventory/SMInventoryContextMenuWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryContextMenuWidget::USMInventoryContextMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , bCanOpenSkillInventory(false)
	  , bCanDropItem(false)
	  , bCanDeleteItem(false)
{
}

void USMInventoryContextMenuWidget::InitializeContextMenu(const FGuid& InItemInstanceId,
                                                          USMInventoryComponent* InInventoryComponent)
{
	ItemInstanceId = InItemInstanceId;
	InventoryComponent = InInventoryComponent;
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (InventoryComponent != nullptr && ItemInstanceId.IsValid())
	{
		bCanOpenSkillInventory = InventoryComponent->FindSkill(ItemInstanceId) != nullptr;
		bCanDropItem = true;
		bCanDeleteItem = true;
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestDropItem()
{
	if (InventoryComponent == nullptr || bCanDropItem == false)
	{
		return;
	}

	if (InventoryComponent->DropItem(ItemInstanceId, FTransform::Identity) == false)
	{
		return;
	}

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->RefreshPanel();
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestOpenSkillInventory()
{
	if (InventoryComponent == nullptr || bCanOpenSkillInventory == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->OpenSkillInventory(ItemInstanceId);
	}
}

void USMInventoryContextMenuWidget::RequestDeleteItem()
{
	if (InventoryComponent == nullptr || bCanDeleteItem == false)
	{
		return;
	}

	if (InventoryComponent->RemoveItem(ItemInstanceId) == false)
	{
		return;
	}

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->RefreshPanel();
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestDetachEmbeddedItem()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	if (InventoryComponent->DetachEmbeddedItem(ItemInstanceId) == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->RefreshPanel();
	}
}
