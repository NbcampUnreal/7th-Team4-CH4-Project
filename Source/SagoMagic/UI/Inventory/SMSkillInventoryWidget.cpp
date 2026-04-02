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
    BP_OnSkillInventoryUpdated();
}

void USMSkillInventoryWidget::ChangeTargetSkill(const FGuid& InSkillInstanceId, const FGuid& InContainerId)
{
    SkillInstanceId = InSkillInstanceId;
    SetContainerId(InContainerId);
    RefreshGrid();
    BP_OnSkillInventoryUpdated();
}

void USMSkillInventoryWidget::ClearTargetSkill()
{
    SkillInstanceId.Invalidate();
    SetContainerId(FGuid());
    RefreshGrid();
    BP_OnSkillInventoryUpdated();
}
