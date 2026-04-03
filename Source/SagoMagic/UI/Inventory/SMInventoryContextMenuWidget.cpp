#include "UI/Inventory/SMInventoryContextMenuWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"

USMInventoryContextMenuWidget::USMInventoryContextMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
{
}

void USMInventoryContextMenuWidget::InitializeContextMenu(const FGuid& InItemInstanceId,
                                                          USMInventoryComponent* InInventoryComponent)
{
	ItemInstanceId = InItemInstanceId;
	InventoryComponent = InInventoryComponent;

	BP_OnContextMenuUpdated();
}

void USMInventoryContextMenuWidget::RequestDropItem()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->DropItem(ItemInstanceId);
}

void USMInventoryContextMenuWidget::RequestDetachEmbeddedItem()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	InventoryComponent->DetachEmbeddedItem(ItemInstanceId);
}
