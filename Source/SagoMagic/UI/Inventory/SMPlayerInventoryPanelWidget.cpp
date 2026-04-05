#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Core/SMContainerTypes.h"

#include "UI/Inventory/SMInventoryGridWidget.h"
#include "UI/Inventory/SMSkillInventoryWidget.h"
#include "UI/Inventory/SMQuickSlotBarWidget.h"
#include "UI/Inventory/SMInventoryContextMenuWidget.h"

USMPlayerInventoryPanelWidget::USMPlayerInventoryPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , MainInventoryGridWidget(nullptr)
	  , SkillInventoryWidget(nullptr)
	  , QuickSlotBarWidget(nullptr)
	  , ContextMenuWidget(nullptr)
{
}

void USMPlayerInventoryPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USMPlayerInventoryPanelWidget::InitializePanelWidget(USMInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;
	InitializeChildWidgets();
	RefreshPanel();
}

void USMPlayerInventoryPanelWidget::RefreshPanel()
{
	ApplySelectedSkillState();
	RefreshMainInventoryWidget();
	RefreshSkillInventoryWidget();
	RefreshQuickSlotBarWidget();

	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::SelectSkill(const FGuid& InSkillInstanceId)
{
	OpenSkillInventory(InSkillInstanceId);
}

void USMPlayerInventoryPanelWidget::ClearSelectedSkill()
{
	CloseSkillInventory();
}

void USMPlayerInventoryPanelWidget::OpenSkillInventory(const FGuid& InSkillInstanceId)
{
	SelectedSkillInstanceId = InSkillInstanceId;
	ApplySelectedSkillState();
	RefreshSkillInventoryWidget();
	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::CloseSkillInventory()
{
	SelectedSkillInstanceId.Invalidate();
	ApplySelectedSkillState();
	RefreshSkillInventoryWidget();
	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::RefreshMainInventoryWidget()
{
	if (MainInventoryGridWidget == nullptr)
	{
		return;
	}

	MainInventoryGridWidget->RefreshGrid();
}

void USMPlayerInventoryPanelWidget::RefreshSkillInventoryWidget()
{
	ApplySelectedSkillState();

	if (SkillInventoryWidget != nullptr)
	{
		SkillInventoryWidget->RefreshGrid();
	}
}

void USMPlayerInventoryPanelWidget::RefreshQuickSlotBarWidget()
{
	if (QuickSlotBarWidget != nullptr)
	{
		QuickSlotBarWidget->RefreshQuickSlotBar();
	}
}

bool USMPlayerInventoryPanelWidget::RequestRotateCurrentDraggedItem()
{
	if (MainInventoryGridWidget != nullptr && MainInventoryGridWidget->GetActiveDragDropOperation() != nullptr)
	{
		MainInventoryGridWidget->RequestRotateDraggedItem();
		return true;
	}

	if (SkillInventoryWidget != nullptr && SkillInventoryWidget->GetActiveDragDropOperation() != nullptr)
	{
		SkillInventoryWidget->RequestRotateDraggedItem();
		return true;
	}

	return false;
}

void USMPlayerInventoryPanelWidget::InitializeChildWidgets()
{
	FGuid MainInventoryContainerId;
	if (InventoryComponent != nullptr)
	{
		MainInventoryContainerId = InventoryComponent->GetMainInventory().ContainerId;
	}

	if (MainInventoryGridWidget != nullptr)
	{
		MainInventoryGridWidget->InitializeGridWidget(MainInventoryContainerId, InventoryComponent);
	}

	if (QuickSlotBarWidget != nullptr)
	{
		QuickSlotBarWidget->InitializeQuickSlotBarWidget(InventoryComponent);
	}

	if (ContextMenuWidget != nullptr)
	{
		ContextMenuWidget->SetInventoryComponent(InventoryComponent);
		ContextMenuWidget->SetItemInstanceId(FGuid());
	}

	ApplySelectedSkillState();
}

void USMPlayerInventoryPanelWidget::ApplySelectedSkillState()
{
	if (SkillInventoryWidget == nullptr)
	{
		return;
	}

	if (InventoryComponent == nullptr)
	{
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	if (SelectedSkillInstanceId.IsValid() == false)
	{
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(SelectedSkillInstanceId, SkillData) == false)
	{
		SkillInventoryWidget->ClearTargetSkill();
		return;
	}

	SkillInventoryWidget->ChangeTargetSkill(SelectedSkillInstanceId, SkillData.InternalContainerId);
}
