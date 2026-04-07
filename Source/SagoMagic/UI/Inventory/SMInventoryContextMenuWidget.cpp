#include "UI/Inventory/SMInventoryContextMenuWidget.h"

#include "Character/SMPlayerController.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryContextMenuWidget::USMInventoryContextMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , OwningPanelWidget(nullptr)
	  , bCanOpenSkillInventory(false)
	  , bCanDropItem(false)
	  , bCanDeleteItem(false)
{
}

bool USMInventoryContextMenuWidget::CanOpenSkillInventory() const
{
	return bCanOpenSkillInventory;
}

bool USMInventoryContextMenuWidget::CanDropItem() const
{
	return bCanDropItem;
}

bool USMInventoryContextMenuWidget::CanDeleteItem() const
{
	return bCanDeleteItem;
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
		FSMSkillItemInstanceData SkillData;
		bCanOpenSkillInventory = InventoryComponent->GetSkillData(ItemInstanceId, SkillData);
		bCanDropItem = InventoryComponent->CanDropItem(ItemInstanceId);
		bCanDeleteItem = InventoryComponent->HasItem(ItemInstanceId);
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestDropItem()
{
	if (InventoryComponent == nullptr || bCanDropItem == false)
	{
		return;
	}

	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCDropInventoryItem(ItemInstanceId);

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		return;
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestOpenSkillInventory()
{
	if (InventoryComponent == nullptr || bCanOpenSkillInventory == false)
	{
		return;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData) == false)
	{
		return;
	}

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->OpenSkillInventory(ItemInstanceId);
		OwningPanelWidget->CloseContextMenu();
		return;
	}
}

void USMInventoryContextMenuWidget::RequestDeleteItem()
{
	if (InventoryComponent == nullptr || bCanDeleteItem == false)
	{
		return;
	}

	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCRemoveInventoryItem(ItemInstanceId);

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		return;
	}

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestDetachEmbeddedItem()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCDetachEmbeddedItem(ItemInstanceId);

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		return;
	}

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;
	BP_OnContextMenuUpdated();
}
