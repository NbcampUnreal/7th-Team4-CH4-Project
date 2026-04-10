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
		Slots.Reset();
		ActiveSlotIndex = 0;
		return;
	}

	const FSMQuickSlotSetState& QuickSlots = InventoryComponent->GetQuickSlots();
	Slots = QuickSlots.Slots;
	Slot1SkillId = QuickSlots.Slot1SkillId;
	Slot2SkillId = QuickSlots.Slot2SkillId;
	ActiveSlotIndex = QuickSlots.ActiveSlotIndex;

	if (Slot1SkillId.IsValid() == false)
	{
		for (const FSMQuickSlotEntry& SlotEntry : Slots)
		{
			if (SlotEntry.GetSlotIndex() == 0)
			{
				Slot1SkillId = SlotEntry.GetEquippedSkillId();
				break;
			}
		}
	}

	if (Slot2SkillId.IsValid() == false)
	{
		for (const FSMQuickSlotEntry& SlotEntry : Slots)
		{
			if (SlotEntry.GetSlotIndex() == 1)
			{
				Slot2SkillId = SlotEntry.GetEquippedSkillId();
				break;
			}
		}
	}
}
