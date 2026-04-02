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
	RefreshMainInventoryWidget();
	RefreshSkillInventoryWidget();
	RefreshQuickSlotBarWidget();

	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::SelectSkill(const FGuid& InSkillInstanceId)
{
	SelectedSkillInstanceId = InSkillInstanceId;
	ApplySelectedSkillState();
	RefreshSkillInventoryWidget();
	BP_OnPanelRefreshed();
}

void USMPlayerInventoryPanelWidget::ClearSelectedSkill()
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
	if (SkillInventoryWidget == nullptr)
	{
		return;
	}

	SkillInventoryWidget->RefreshGrid();
}

void USMPlayerInventoryPanelWidget::RefreshQuickSlotBarWidget()
{
	if (QuickSlotBarWidget == nullptr)
	{
		return;
	}

	QuickSlotBarWidget->RefreshQuickSlotBar();
}

void USMPlayerInventoryPanelWidget::InitializeChildWidgets()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	if (MainInventoryGridWidget != nullptr)
	{
		MainInventoryGridWidget->InitializeGridWidget(
			InventoryComponent->GetMainInventory().ContainerId,
			InventoryComponent);
	}

	if (QuickSlotBarWidget != nullptr)
	{
		QuickSlotBarWidget->InitializeQuickSlotBarWidget(InventoryComponent);
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
