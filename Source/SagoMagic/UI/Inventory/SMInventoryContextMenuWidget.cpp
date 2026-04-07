#include "UI/Inventory/SMInventoryContextMenuWidget.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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

	FTransform DropTransform = FTransform::Identity;

	if (APlayerController* OwningPlayerController = GetOwningPlayer())
	{
		if (APawn* OwningPawn = OwningPlayerController->GetPawn())
		{
			DropTransform = FTransform(
				OwningPawn->GetActorRotation(),
				OwningPawn->GetActorLocation(),
				FVector::OneVector);
		}
	}

	if (InventoryComponent->DropItem(ItemInstanceId, DropTransform) == false)
	{
		return;
	}

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		OwningPanelWidget->RefreshPanel();
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

	if (InventoryComponent->RemoveItem(ItemInstanceId) == false)
	{
		return;
	}

	ItemInstanceId.Invalidate();
	bCanOpenSkillInventory = false;
	bCanDropItem = false;
	bCanDeleteItem = false;

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		OwningPanelWidget->RefreshPanel();
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

	if (InventoryComponent->DetachEmbeddedItem(ItemInstanceId) == false)
	{
		return;
	}

	if (OwningPanelWidget != nullptr)
	{
		OwningPanelWidget->CloseContextMenu();
		OwningPanelWidget->RefreshPanel();
		return;
	}

	BP_OnContextMenuUpdated();
}
