#include "UI/Inventory/SMQuickSlotBarWidget.h"

#include "Character/SMPlayerController.h"
#include "GameplayTags/Message/SMMessageTag.h"
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

void USMQuickSlotBarWidget::NativeDestruct()
{
	UnregisterQuickSlotMessageListener();
	Super::NativeDestruct();
}

void USMQuickSlotBarWidget::InitializeQuickSlotBarWidget(USMInventoryComponent* InInventoryComponent)
{
	UnregisterQuickSlotMessageListener();
	InventoryComponent = InInventoryComponent;
	RegisterQuickSlotMessageListener();
	RefreshQuickSlotBar();
}

void USMQuickSlotBarWidget::RefreshQuickSlotBar()
{
	SyncFromInventoryComponent();
	BP_OnQuickSlotBarUpdated();
}

void USMQuickSlotBarWidget::ActivateFirstSlot()
{
	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCSetActiveQuickSlot(0);
}

void USMQuickSlotBarWidget::ActivateSecondSlot()
{
	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCSetActiveQuickSlot(1);
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

void USMQuickSlotBarWidget::RegisterQuickSlotMessageListener()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	QuickSlotUpdatedListenerHandle = MessageSubsystem.RegisterListener<FSMQuickSlotUpdatedMessage>(
		SMMessageTag::Inventory_QuickSlotUpdated,
		this,
		&ThisClass::HandleQuickSlotUpdatedMessage);
}

void USMQuickSlotBarWidget::UnregisterQuickSlotMessageListener()
{
	if (QuickSlotUpdatedListenerHandle.IsValid())
	{
		QuickSlotUpdatedListenerHandle.Unregister();
	}
}

void USMQuickSlotBarWidget::HandleQuickSlotUpdatedMessage(
	FGameplayTag InChannel,
	const FSMQuickSlotUpdatedMessage& InMessage)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	APlayerState* OwningPlayerState = OwningPlayerController->GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr || InMessage.GetOwningPlayerState() != OwningPlayerState)
	{
		return;
	}

	RefreshQuickSlotBar();
}
