#include "UI/Inventory/SMQuickSlotBarWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"

USMQuickSlotBarWidget::USMQuickSlotBarWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , InventoryComponent(nullptr)
    , ActiveSlotIndex(0)
{
}

void USMQuickSlotBarWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void USMQuickSlotBarWidget::InitializeQuickSlotBarWidget(USMInventoryComponent* InInventoryComponent)
{
    InventoryComponent = InInventoryComponent;
    RefreshQuickSlotBar();
}

void USMQuickSlotBarWidget::RefreshQuickSlotBar()
{
    SyncFromInventoryComponent();
    BP_OnQuickSlotBarUpdated();
}

void USMQuickSlotBarWidget::ActivateFirstSlot()
{
    if (InventoryComponent == nullptr)
    {
        return;
    }

    InventoryComponent->SetActiveQuickSlot(0);
    RefreshQuickSlotBar();
}

void USMQuickSlotBarWidget::ActivateSecondSlot()
{
    if (InventoryComponent == nullptr)
    {
        return;
    }

    InventoryComponent->SetActiveQuickSlot(1);
    RefreshQuickSlotBar();
}

void USMQuickSlotBarWidget::SyncFromInventoryComponent()
{
    if (InventoryComponent == nullptr)
    {
        Slot1SkillId.Invalidate();
        Slot2SkillId.Invalidate();
        ActiveSlotIndex = 0;
        return;
    }

    const FSMQuickSlotSetState& QuickSlots = InventoryComponent->GetQuickSlots();
    Slot1SkillId = QuickSlots.Slot1SkillId;
    Slot2SkillId = QuickSlots.Slot2SkillId;
    ActiveSlotIndex = QuickSlots.ActiveSlotIndex;
}
