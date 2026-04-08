#include "UI/Inventory/SMSkillInventoryWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"

USMSkillInventoryWidget::USMSkillInventoryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USMSkillInventoryWidget::InitializeSkillInventoryWidget(
	const FGuid& InSkillInstanceId,
	const FGuid& InContainerId,
	USMInventoryComponent* InInventoryComponent)
{
	SkillInstanceId = InSkillInstanceId;
	InitializeGridWidget(InContainerId, InInventoryComponent);
	SetVisibility(InSkillInstanceId.IsValid() && InContainerId.IsValid() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	BP_OnSkillInventoryUpdated();
}

void USMSkillInventoryWidget::ChangeTargetSkill(const FGuid& InSkillInstanceId, const FGuid& InContainerId)
{
	const bool bTargetUnchanged =
		SkillInstanceId == InSkillInstanceId &&
		GetContainerId() == InContainerId;

	SkillInstanceId = InSkillInstanceId;
	SetContainerId(InContainerId);

	if (InSkillInstanceId.IsValid() == false || InContainerId.IsValid() == false)
	{
		ClearTargetSkill();
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (bTargetUnchanged == false)
	{
		RefreshGrid();
	}

	BP_OnSkillInventoryUpdated();
}

void USMSkillInventoryWidget::ClearTargetSkill()
{
	const bool bAlreadyCleared =
		SkillInstanceId.IsValid() == false &&
		GetContainerId().IsValid() == false &&
		GridWidth == 0 &&
		GridHeight == 0;

	if (bAlreadyCleared)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		BP_OnSkillInventoryUpdated();
		return;
	}

	SkillInstanceId.Invalidate();
	SetContainerId(FGuid());
	GridWidth = 0;
	GridHeight = 0;
	ClearActiveDragState();
	ClearItemWidgets();
	ClearCellWidgets();
	SetVisibility(ESlateVisibility::Collapsed);
	BP_OnGridRefreshed();
	BP_OnSkillInventoryUpdated();
}
