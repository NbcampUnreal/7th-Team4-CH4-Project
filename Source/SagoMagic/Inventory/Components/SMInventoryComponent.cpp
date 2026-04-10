#include "Inventory/Components/SMInventoryComponent.h"

#include "Containers/BitArray.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include "Core/DataManager/SMSyncDataManager.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"

#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Definitions/SMSkillItemDefinition.h"
#include "Inventory/Items/Definitions/SMGemItemDefinition.h"

#include "Inventory/Items/Fragments/SMGridShapeFragment.h"
#include "Inventory/Items/Fragments/SMAbilityFragment.h"
#include "Inventory/Items/Fragments/SMGemModifierFragment.h"
#include "Inventory/Items/Fragments/SMInternalInventoryFragment.h"
#include "Inventory/Items/Fragments/SMSkillProgressionFragment.h"
#include "Inventory/Items/Fragments/SMDropRuleFragment.h"
#include "Inventory/Items/Fragments/SMWorldVisualFragment.h"

#include "GameplayTags/Message/SMMessageTag.h"

#include "Inventory/World/SMBaseItemDropActor.h"
#include "GameplayTags/Character/SMSkillTag.h"

USMInventoryComponent::USMInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	DefaultMainInventoryMask.Width = 15;
	DefaultMainInventoryMask.Height = 15;
	DefaultMainInventoryMask.BitMask = FString::ChrN(DefaultMainInventoryMask.Width * DefaultMainInventoryMask.Height, TEXT('1'));
}

void USMInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() == nullptr)
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		InitializeMainInventory();
		InitializeQuickSlots();
	}
}

void USMInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// PlayerState에 붙어있지만 실제 인벤토리 상태는 소유 클라이언트에게만 보냄
	DOREPLIFETIME_CONDITION(USMInventoryComponent, MainInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USMInventoryComponent, QuickSlots, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USMInventoryComponent, ItemEntries, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USMInventoryComponent, SkillEntries, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USMInventoryComponent, SkillInternalContainers, COND_OwnerOnly);
}

FGuid USMInventoryComponent::AddItemFromDefinition(const TSoftObjectPtr<USMItemDefinition>& InItemDefinition)
{
	if (GetOwner() == nullptr)
	{
		return FGuid();
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return FGuid();
	}

	if (InItemDefinition.IsNull())
	{
		return FGuid();
	}

	const USMItemDefinition* ItemDefinition = InItemDefinition.LoadSynchronous();
	if (ItemDefinition == nullptr)
	{
		return FGuid();
	}

	ESMItemType ResolvedItemType = ESMItemType::None;

	if (ItemDefinition->IsA<USMSkillItemDefinition>())
	{
		ResolvedItemType = ESMItemType::Skill;
	}
	else if (ItemDefinition->IsA<USMGemItemDefinition>())
	{
		ResolvedItemType = ESMItemType::Gem;
	}
	else
	{
		return FGuid();
	}

	FSMItemDropPayload DropPayload;
	DropPayload.SetInstanceId(FGuid::NewGuid());
	DropPayload.ItemType = ResolvedItemType;
	DropPayload.SetDefinition(InItemDefinition);
	DropPayload.SetRotation(ESMGridRotation::Rot0);
	DropPayload.SetLocked(false);

	return AddItemFromDropPayload(DropPayload);
}

void USMInventoryComponent::ResetInventory()
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return;
	}

	MainInventory = FSMGridContainerState();
	QuickSlots = FSMQuickSlotSetState();
	ItemEntries.Reset();
	SkillEntries.Reset();
	SkillInternalContainers.Reset();

	InitializeMainInventory();
	InitializeQuickSlots();

	PublishInventoryUpdatedMessage(MainInventory.ContainerId);
	PublishQuickSlotUpdatedMessage(0);
	PublishQuickSlotUpdatedMessage(1);
}

FGuid USMInventoryComponent::AddItemFromDropPayload(const FSMItemDropPayload& InDropPayload)
{
	if (GetOwner() == nullptr)
	{
		return FGuid();
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return FGuid();
	}

	if (InDropPayload.IsValidPayload() == false)
	{
		return FGuid();
	}

	if (MainInventory.IsValidContainer() == false)
	{
		InitializeMainInventory();
	}

	const USMItemDefinition* ItemDefinition = InDropPayload.GetDefinition().LoadSynchronous();
	if (ItemDefinition == nullptr)
	{
		return FGuid();
	}

	if (InDropPayload.ItemType == ESMItemType::Skill)
	{
		const USMInternalInventoryFragment* InternalInventoryFragment =
			ItemDefinition->FindFragmentByClass<USMInternalInventoryFragment>();
		if (InternalInventoryFragment == nullptr)
		{
			return FGuid();
		}

		FSMSkillItemInstanceData NewSkillEntry;
		NewSkillEntry.BaseItem.InstanceId = InDropPayload.GetInstanceId();
		NewSkillEntry.BaseItem.ItemType = ESMItemType::Skill;
		NewSkillEntry.BaseItem.Definition = InDropPayload.GetDefinition();
		NewSkillEntry.BaseItem.ParentContainerId = MainInventory.ContainerId;
		NewSkillEntry.BaseItem.GridX = 0;
		NewSkillEntry.BaseItem.GridY = 0;
		NewSkillEntry.BaseItem.Rotation = InDropPayload.GetRotation();
		NewSkillEntry.BaseItem.bLocked = InDropPayload.IsLocked();

		FGuid InternalContainerId;
		if (CreateSkillInternalContainer(
			NewSkillEntry.BaseItem.InstanceId,
			InternalInventoryFragment->GetInternalMask(),
			InternalContainerId) == false)
		{
			return FGuid();
		}

		NewSkillEntry.InternalContainerId = InternalContainerId;
		NewSkillEntry.EmbeddedItemIds.Reset();

		SkillEntries.Add(NewSkillEntry);

		int32 FoundGridX = 0;
		int32 FoundGridY = 0;

		if (FindAvailablePosition(
			NewSkillEntry.BaseItem.InstanceId,
			MainInventory.ContainerId,
			FoundGridX,
			FoundGridY) == false)
		{
			SkillEntries.RemoveAll(
				[&](const FSMSkillItemInstanceData& Entry)
				{
					return Entry.BaseItem.InstanceId == NewSkillEntry.BaseItem.InstanceId;
				});

			SkillInternalContainers.RemoveAll(
				[&](const FSMGridContainerState& ContainerState)
				{
					return ContainerState.ContainerId == InternalContainerId;
				});

			return FGuid();
		}

		FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(NewSkillEntry.BaseItem.InstanceId);
		if (EditableSkill == nullptr)
		{
			SkillEntries.RemoveAll(
				[&](const FSMSkillItemInstanceData& Entry)
				{
					return Entry.BaseItem.InstanceId == NewSkillEntry.BaseItem.InstanceId;
				});

			SkillInternalContainers.RemoveAll(
				[&](const FSMGridContainerState& ContainerState)
				{
					return ContainerState.ContainerId == InternalContainerId;
				});

			return FGuid();
		}

		EditableSkill->BaseItem.GridX = FoundGridX;
		EditableSkill->BaseItem.GridY = FoundGridY;

		MainInventory.ContainedItemIds.Add(EditableSkill->BaseItem.InstanceId);

		if (RestoreNestedPayloads(InDropPayload, EditableSkill->BaseItem.InstanceId) == false)
		{
			RemoveItemInternal(EditableSkill->BaseItem.InstanceId, false);
			return FGuid();
		}

		PublishInventoryUpdatedMessage(MainInventory.ContainerId);
		PublishInventoryUpdatedMessage(EditableSkill->InternalContainerId);
		return EditableSkill->BaseItem.InstanceId;
	}

	FSMItemInstanceData NewItemEntry;
	NewItemEntry.InstanceId = InDropPayload.GetInstanceId();
	NewItemEntry.ItemType = InDropPayload.ItemType;
	NewItemEntry.Definition = InDropPayload.GetDefinition();
	NewItemEntry.ParentContainerId = MainInventory.ContainerId;
	NewItemEntry.GridX = 0;
	NewItemEntry.GridY = 0;
	NewItemEntry.Rotation = InDropPayload.GetRotation();
	NewItemEntry.bLocked = InDropPayload.IsLocked();

	ItemEntries.Add(NewItemEntry);

	int32 FoundGridX = 0;
	int32 FoundGridY = 0;

	if (FindAvailablePosition(
		NewItemEntry.InstanceId,
		MainInventory.ContainerId,
		FoundGridX,
		FoundGridY) == false)
	{
		ItemEntries.RemoveAll(
			[&](const FSMItemInstanceData& Entry)
			{
				return Entry.InstanceId == NewItemEntry.InstanceId;
			});

		return FGuid();
	}

	FSMItemInstanceData* EditableItem = FindEditableItem(NewItemEntry.InstanceId);
	if (EditableItem == nullptr)
	{
		ItemEntries.RemoveAll(
			[&](const FSMItemInstanceData& Entry)
			{
				return Entry.InstanceId == NewItemEntry.InstanceId;
			});

		return FGuid();
	}

	EditableItem->GridX = FoundGridX;
	EditableItem->GridY = FoundGridY;

	MainInventory.ContainedItemIds.Add(NewItemEntry.InstanceId);
	PublishInventoryUpdatedMessage(MainInventory.ContainerId);
	return EditableItem->InstanceId;
}

bool USMInventoryComponent::RemoveItem(const FGuid& InItemInstanceId)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	return RemoveItemInternal(InItemInstanceId, true);
}

bool USMInventoryComponent::DropItem(const FGuid& InItemInstanceId, const FTransform& InDropTransform)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (CanDropItemInternal(InItemInstanceId) == false)
	{
		return false;
	}

	FSMItemDropPayload DropPayload;
	if (BuildDropPayload(InItemInstanceId, DropPayload) == false)
	{
		return false;
	}

	ASMBaseItemDropActor* SpawnedDropActor = SpawnDropActorFromPayload(DropPayload, InDropTransform);
	if (SpawnedDropActor == nullptr)
	{
		return false;
	}

	if (RemoveItemInternal(InItemInstanceId, true) == false)
	{
		SpawnedDropActor->Destroy();
		return false;
	}

	return true;
}

bool USMInventoryComponent::MoveItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX,
                                     int32 InGridY, ESMGridRotation InRotation)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	FSMGridContainerState* TargetContainer = FindEditableContainer(InTargetContainerId);
	if (TargetContainer == nullptr)
	{
		return false;
	}

	FSMItemInstanceData* EditableItem = FindEditableItem(InItemInstanceId);
	FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(InItemInstanceId);

	const bool bIsNormalItem = EditableItem != nullptr;
	const bool bIsSkillItem = EditableSkill != nullptr;

	if (bIsNormalItem == false && bIsSkillItem == false)
	{
		return false;
	}

	const ESMItemType MovingItemType = bIsNormalItem
		                                   ? EditableItem->ItemType
		                                   : EditableSkill->BaseItem.ItemType;

	const FGuid PreviousContainerId = bIsNormalItem
		                                  ? EditableItem->ParentContainerId
		                                  : EditableSkill->BaseItem.ParentContainerId;

	const int32 PreviousGridX = bIsNormalItem
		                            ? EditableItem->GridX
		                            : EditableSkill->BaseItem.GridX;

	const int32 PreviousGridY = bIsNormalItem
		                            ? EditableItem->GridY
		                            : EditableSkill->BaseItem.GridY;

	const ESMGridRotation PreviousRotation = bIsNormalItem
		                                         ? EditableItem->Rotation
		                                         : EditableSkill->BaseItem.Rotation;

	if (PreviousContainerId == InTargetContainerId &&
		PreviousGridX == InGridX &&
		PreviousGridY == InGridY &&
		PreviousRotation == InRotation)
	{
		return true;
	}

	const FSMGridContainerState* PreviousContainer = FindContainer(PreviousContainerId);
	if (PreviousContainer == nullptr)
	{
		return false;
	}

	FGuid PreviousOwningSkillId;
	for (const FSMSkillItemInstanceData& SkillEntry : SkillEntries)
	{
		if (SkillEntry.InternalContainerId == PreviousContainerId)
		{
			PreviousOwningSkillId = SkillEntry.BaseItem.InstanceId;
			break;
		}
	}

	FGuid TargetOwningSkillId;

	if (TargetContainer->ContainerType == ESMContainerType::SkillInternal)
	{
		const FSMSkillItemInstanceData* TargetOwningSkill = nullptr;
		for (const FSMSkillItemInstanceData& SkillEntry : SkillEntries)
		{
			if (SkillEntry.InternalContainerId == InTargetContainerId)
			{
				TargetOwningSkill = &SkillEntry;
				break;
			}
		}

		if (TargetOwningSkill == nullptr)
		{
			return false;
		}

		TargetOwningSkillId = TargetOwningSkill->BaseItem.InstanceId;

		if (TargetOwningSkill->BaseItem.InstanceId == InItemInstanceId)
		{
			return false;
		}

		const USMItemDefinition* TargetSkillDefinition = ResolveItemDefinition(TargetOwningSkill->BaseItem);
		if (TargetSkillDefinition == nullptr)
		{
			return false;
		}

		const USMInternalInventoryFragment* TargetInternalInventoryFragment =
			TargetSkillDefinition->FindFragmentByClass<USMInternalInventoryFragment>();
		if (TargetInternalInventoryFragment == nullptr)
		{
			return false;
		}

		if (MovingItemType == ESMItemType::Gem)
		{
			if (bIsNormalItem == false)
			{
				return false;
			}

			const USMItemDefinition* GemDefinition = ResolveItemDefinition(*EditableItem);
			if (GemDefinition == nullptr)
			{
				return false;
			}

			const USMGemModifierFragment* GemModifierFragment =
				GemDefinition->FindFragmentByClass<USMGemModifierFragment>();
			if (GemModifierFragment == nullptr)
			{
				return false;
			}

			if (TargetInternalInventoryFragment->IsGemAllowed() == false)
			{
				return false;
			}

			if (CanApplyGemToSkillByTags(GemModifierFragment, TargetSkillDefinition) == false)
			{
				return false;
			}
		}
		else if (MovingItemType == ESMItemType::Skill)
		{
			if (bIsSkillItem == false)
			{
				return false;
			}

			if (TargetInternalInventoryFragment->IsSameNamedEmptySkillAllowed() == false)
			{
				return false;
			}

			if (IsSameNamedSkill(InItemInstanceId, TargetOwningSkill->BaseItem.InstanceId) == false)
			{
				return false;
			}

			if (IsSkillActuallyEmpty(InItemInstanceId) == false)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (CanPlaceItem(InItemInstanceId, InTargetContainerId, InGridX, InGridY, InRotation) == false)
	{
		return false;
	}

	FSMGridContainerState* PreviousEditableContainer = FindEditableContainer(PreviousContainerId);
	if (PreviousEditableContainer != nullptr)
	{
		PreviousEditableContainer->ContainedItemIds.Remove(InItemInstanceId);
	}

	for (FSMSkillItemInstanceData& SkillEntry : SkillEntries)
	{
		if (SkillEntry.InternalContainerId == PreviousContainerId)
		{
			SkillEntry.EmbeddedItemIds.Remove(InItemInstanceId);
			break;
		}
	}

	if (bIsNormalItem)
	{
		EditableItem->ParentContainerId = InTargetContainerId;
		EditableItem->GridX = InGridX;
		EditableItem->GridY = InGridY;
		EditableItem->Rotation = InRotation;
	}
	else
	{
		EditableSkill->BaseItem.ParentContainerId = InTargetContainerId;
		EditableSkill->BaseItem.GridX = InGridX;
		EditableSkill->BaseItem.GridY = InGridY;
		EditableSkill->BaseItem.Rotation = InRotation;
	}

	TargetContainer->ContainedItemIds.AddUnique(InItemInstanceId);

	for (FSMSkillItemInstanceData& SkillEntry : SkillEntries)
	{
		if (SkillEntry.InternalContainerId == InTargetContainerId)
		{
			SkillEntry.EmbeddedItemIds.AddUnique(InItemInstanceId);
			break;
		}
	}

	if (PreviousOwningSkillId.IsValid())
	{
		RebuildSkillSummary(PreviousOwningSkillId);
	}

	if (TargetOwningSkillId.IsValid() && TargetOwningSkillId != PreviousOwningSkillId)
	{
		RebuildSkillSummary(TargetOwningSkillId);
	}

	if (PreviousContainerId != InTargetContainerId)
	{
		PublishInventoryUpdatedMessage(PreviousContainerId);
	}

	PublishInventoryUpdatedMessage(InTargetContainerId);
	return true;
}

bool USMInventoryComponent::CanPlaceItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX,
                                         int32 InGridY, ESMGridRotation InRotation) const
{
	const FSMGridContainerState* TargetContainer = FindContainer(InTargetContainerId);
	if (TargetContainer == nullptr)
	{
		return false;
	}

	TArray<FIntPoint> OccupiedCells;
	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, InRotation, OccupiedCells) == false)
	{
		return false;
	}

	for (const FIntPoint& Cell : OccupiedCells)
	{
		if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= TargetContainer->ValidMask.Width || Cell.Y >= TargetContainer->
			ValidMask.Height)
		{
			return false;
		}

		const int32 MaskIndex = (Cell.Y * TargetContainer->ValidMask.Width) + Cell.X;
		if (TargetContainer->ValidMask.BitMask.IsValidIndex(MaskIndex) == false)
		{
			return false;
		}

		if (TargetContainer->ValidMask.BitMask[MaskIndex] != TEXT('1'))
		{
			return false;
		}
	}

	if (HasPlacementConflict(InItemInstanceId, InTargetContainerId, InGridX, InGridY, InRotation))
	{
		return false;
	}

	return true;
}

bool USMInventoryComponent::FindAvailablePosition(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId,
                                                  int32& OutGridX, int32& OutGridY) const
{
	const FSMGridContainerState* TargetContainer = FindContainer(InTargetContainerId);
	if (TargetContainer == nullptr)
	{
		return false;
	}

	ESMGridRotation RotationToUse = ESMGridRotation::Rot0;

	if (const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId))
	{
		RotationToUse = ItemData->Rotation;
	}
	else if (const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId))
	{
		RotationToUse = SkillData->BaseItem.Rotation;
	}
	else
	{
		return false;
	}

	TArray<FIntPoint> ShapeCells;
	if (BuildOccupiedCells(InItemInstanceId, 0, 0, RotationToUse, ShapeCells) == false)
	{
		return false;
	}

	TBitArray<> OccupiedBits;
	OccupiedBits.Init(false, TargetContainer->ValidMask.Width * TargetContainer->ValidMask.Height);

	for (const FSMItemInstanceData& Entry : ItemEntries)
	{
		if (Entry.InstanceId == InItemInstanceId || Entry.ParentContainerId != InTargetContainerId)
		{
			continue;
		}

		AddOccupiedBits(OccupiedBits, *TargetContainer, Entry.InstanceId, Entry.GridX, Entry.GridY, Entry.Rotation);
	}

	for (const FSMSkillItemInstanceData& Entry : SkillEntries)
	{
		if (Entry.BaseItem.InstanceId == InItemInstanceId || Entry.BaseItem.ParentContainerId != InTargetContainerId)
		{
			continue;
		}

		AddOccupiedBits(OccupiedBits, *TargetContainer, Entry.BaseItem.InstanceId, Entry.BaseItem.GridX, Entry.BaseItem.GridY, Entry.BaseItem.Rotation);
	}

	for (int32 Y = 0; Y < TargetContainer->ValidMask.Height; ++Y)
	{
		for (int32 X = 0; X < TargetContainer->ValidMask.Width; ++X)
		{
			bool bCanPlace = true;

			for (const FIntPoint& ShapeCell : ShapeCells)
			{
				const int32 CellX = X + ShapeCell.X;
				const int32 CellY = Y + ShapeCell.Y;
				if (CellX < 0 || CellY < 0 || CellX >= TargetContainer->ValidMask.Width || CellY >= TargetContainer->ValidMask.Height)
				{
					bCanPlace = false;
					break;
				}

				const int32 CellIndex = (CellY * TargetContainer->ValidMask.Width) + CellX;
				if (TargetContainer->ValidMask.BitMask.IsValidIndex(CellIndex) == false ||
					TargetContainer->ValidMask.BitMask[CellIndex] != TEXT('1') ||
					OccupiedBits[CellIndex])
				{
					bCanPlace = false;
					break;
				}
			}

			if (bCanPlace)
			{
				OutGridX = X;
				OutGridY = Y;
				return true;
			}
		}
	}

	return false;
}

bool USMInventoryComponent::AttachGemToSkill(const FGuid& InGemInstanceId, const FGuid& InTargetSkillInstanceId,
                                             int32 InGridX, int32 InGridY)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	FSMItemInstanceData* EditableGemItem = FindEditableItem(InGemInstanceId);
	if (EditableGemItem == nullptr)
	{
		return false;
	}

	if (EditableGemItem->ItemType != ESMItemType::Gem)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableTargetSkill = FindEditableSkill(InTargetSkillInstanceId);
	if (EditableTargetSkill == nullptr)
	{
		return false;
	}

	const USMItemDefinition* GemDefinition = ResolveItemDefinition(*EditableGemItem);
	if (GemDefinition == nullptr)
	{
		return false;
	}

	const USMItemDefinition* TargetSkillDefinition = ResolveItemDefinition(EditableTargetSkill->BaseItem);
	if (TargetSkillDefinition == nullptr)
	{
		return false;
	}

	const USMGemModifierFragment* GemModifierFragment = GemDefinition->FindFragmentByClass<USMGemModifierFragment>();
	if (GemModifierFragment == nullptr)
	{
		return false;
	}

	const USMInternalInventoryFragment* TargetInternalInventoryFragment =
		TargetSkillDefinition->FindFragmentByClass<USMInternalInventoryFragment>();
	if (TargetInternalInventoryFragment == nullptr)
	{
		return false;
	}

	if (TargetInternalInventoryFragment->IsGemAllowed() == false)
	{
		return false;
	}

	if (CanApplyGemToSkillByTags(GemModifierFragment, TargetSkillDefinition) == false)
	{
		return false;
	}

	return MoveItem(
		InGemInstanceId,
		EditableTargetSkill->InternalContainerId,
		InGridX,
		InGridY,
		EditableGemItem->Rotation);
}

bool USMInventoryComponent::AttachSkillToSkill(const FGuid& InMaterialSkillInstanceId,
                                               const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (InMaterialSkillInstanceId == InTargetSkillInstanceId)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableMaterialSkill = FindEditableSkill(InMaterialSkillInstanceId);
	FSMSkillItemInstanceData* EditableTargetSkill = FindEditableSkill(InTargetSkillInstanceId);

	if (EditableMaterialSkill == nullptr || EditableTargetSkill == nullptr)
	{
		return false;
	}

	const USMItemDefinition* TargetSkillDefinition = ResolveItemDefinition(EditableTargetSkill->BaseItem);
	if (TargetSkillDefinition == nullptr)
	{
		return false;
	}

	const USMInternalInventoryFragment* TargetInternalInventoryFragment =
		TargetSkillDefinition->FindFragmentByClass<USMInternalInventoryFragment>();
	if (TargetInternalInventoryFragment == nullptr)
	{
		return false;
	}

	if (TargetInternalInventoryFragment->IsSameNamedEmptySkillAllowed() == false)
	{
		return false;
	}

	if (IsSameNamedSkill(InMaterialSkillInstanceId, InTargetSkillInstanceId) == false)
	{
		return false;
	}

	if (IsSkillActuallyEmpty(InMaterialSkillInstanceId) == false)
	{
		return false;
	}

	return MoveItem(
		InMaterialSkillInstanceId,
		EditableTargetSkill->InternalContainerId,
		InGridX,
		InGridY,
		EditableMaterialSkill->BaseItem.Rotation);
}

bool USMInventoryComponent::DetachEmbeddedItem(const FGuid& InEmbeddedItemInstanceId)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	FGuid CurrentParentContainerId;
	ESMGridRotation CurrentRotation = ESMGridRotation::Rot0;

	if (const FSMItemInstanceData* ItemData = FindItem(InEmbeddedItemInstanceId))
	{
		CurrentParentContainerId = ItemData->ParentContainerId;
		CurrentRotation = ItemData->Rotation;
	}
	else if (const FSMSkillItemInstanceData* SkillData = FindSkill(InEmbeddedItemInstanceId))
	{
		CurrentParentContainerId = SkillData->BaseItem.ParentContainerId;
		CurrentRotation = SkillData->BaseItem.Rotation;
	}
	else
	{
		return false;
	}

	const FSMGridContainerState* CurrentParentContainer = FindContainer(CurrentParentContainerId);
	if (CurrentParentContainer == nullptr)
	{
		return false;
	}

	if (CurrentParentContainer->ContainerType != ESMContainerType::SkillInternal)
	{
		return false;
	}

	FSMSkillItemInstanceData* ParentSkill = nullptr;
	for (FSMSkillItemInstanceData& SkillEntry : SkillEntries)
	{
		if (SkillEntry.InternalContainerId == CurrentParentContainerId)
		{
			ParentSkill = &SkillEntry;
			break;
		}
	}

	if (ParentSkill == nullptr)
	{
		return false;
	}

	int32 FoundGridX = 0;
	int32 FoundGridY = 0;
	if (FindAvailablePosition(InEmbeddedItemInstanceId, MainInventory.ContainerId, FoundGridX, FoundGridY) == false)
	{
		return false;
	}

	return MoveItem(
		InEmbeddedItemInstanceId,
		MainInventory.ContainerId,
		FoundGridX,
		FoundGridY,
		CurrentRotation);
}

bool USMInventoryComponent::EquipSkillToQuickSlot(const FGuid& InSkillInstanceId, int32 InSlotIndex)
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (IsValidQuickSlotIndex(InSlotIndex) == false)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(InSkillInstanceId);
	if (EditableSkill == nullptr)
	{
		return false;
	}

	FSMQuickSlotEntry* TargetSlot = FindEditableQuickSlotEntry(InSlotIndex);
	if (TargetSlot == nullptr || TargetSlot->IsValidSlot() == false)
	{
		return false;
	}

	if (TargetSlot->GetEquippedSkillId() == InSkillInstanceId)
	{
		SetActiveQuickSlot(InSlotIndex);
		return true;
	}

	if (TargetSlot->GetEquippedSkillId().IsValid())
	{
		return false;
	}

	const FGuid PreviousContainerId = EditableSkill->BaseItem.ParentContainerId;
	const FSMGridContainerState* PreviousContainer = FindContainer(PreviousContainerId);
	FSMQuickSlotEntry* PreviousSlot = nullptr;

	for (FSMQuickSlotEntry& SlotEntry : QuickSlots.Slots)
	{
		if (SlotEntry.GetEquippedSkillId() == InSkillInstanceId)
		{
			PreviousSlot = &SlotEntry;
			break;
		}
	}

	if (PreviousContainer != nullptr)
	{
		if (PreviousContainer->ContainerType == ESMContainerType::SkillInternal)
		{
			return false;
		}

		if (FSMGridContainerState* PreviousEditableContainer = FindEditableContainer(PreviousContainerId))
		{
			PreviousEditableContainer->ContainedItemIds.Remove(InSkillInstanceId);
		}
	}
	else if (PreviousSlot == nullptr)
	{
		return false;
	}

	if (PreviousSlot != nullptr)
	{
		PreviousSlot->SetEquippedSkillId(FGuid());

		if (PreviousSlot->GetSlotIndex() == 0)
		{
			QuickSlots.Slot1SkillId.Invalidate();
		}
		else if (PreviousSlot->GetSlotIndex() == 1)
		{
			QuickSlots.Slot2SkillId.Invalidate();
		}
	}

	EditableSkill->BaseItem.ParentContainerId = TargetSlot->GetContainerId();
	EditableSkill->BaseItem.GridX = 0;
	EditableSkill->BaseItem.GridY = 0;

	TargetSlot->SetEquippedSkillId(InSkillInstanceId);
	if (InSlotIndex == 0)
	{
		QuickSlots.Slot1SkillId = InSkillInstanceId;
	}
	else if (InSlotIndex == 1)
	{
		QuickSlots.Slot2SkillId = InSkillInstanceId;
	}

	if (PreviousContainer != nullptr)
	{
		PublishInventoryUpdatedMessage(PreviousContainerId);
	}

	if (PreviousSlot != nullptr)
	{
		PublishQuickSlotUpdatedMessage(PreviousSlot->GetSlotIndex());
	}

	PublishQuickSlotUpdatedMessage(InSlotIndex);
	return true;
}

bool USMInventoryComponent::EquipSkillToFirstAvailableQuickSlot(const FGuid& InSkillInstanceId)
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (FindSkill(InSkillInstanceId) == nullptr)
	{
		return false;
	}

	for (const FSMQuickSlotEntry& SlotEntry : QuickSlots.Slots)
	{
		if (SlotEntry.GetEquippedSkillId() == InSkillInstanceId)
		{
			return false;
		}
	}

	for (int32 SlotIndex = 0; SlotIndex <= 1; ++SlotIndex)
	{
		const FSMQuickSlotEntry* SlotEntry = FindQuickSlotEntry(SlotIndex);
		if (SlotEntry == nullptr || SlotEntry->GetEquippedSkillId().IsValid())
		{
			continue;
		}

		return EquipSkillToQuickSlot(InSkillInstanceId, SlotIndex);
	}

	return false;
}

bool USMInventoryComponent::UnequipSkillFromQuickSlot(int32 InSlotIndex)
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (IsValidQuickSlotIndex(InSlotIndex) == false)
	{
		return false;
	}

	FSMQuickSlotEntry* SlotEntry = FindEditableQuickSlotEntry(InSlotIndex);
	if (SlotEntry == nullptr || SlotEntry->GetEquippedSkillId().IsValid() == false)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(SlotEntry->GetEquippedSkillId());
	if (EditableSkill == nullptr)
	{
		return false;
	}

	int32 FoundGridX = 0;
	int32 FoundGridY = 0;
	if (FindAvailablePosition(EditableSkill->BaseItem.InstanceId, MainInventory.ContainerId, FoundGridX, FoundGridY) == false)
	{
		return false;
	}

	EditableSkill->BaseItem.ParentContainerId = MainInventory.ContainerId;
	EditableSkill->BaseItem.GridX = FoundGridX;
	EditableSkill->BaseItem.GridY = FoundGridY;

	MainInventory.ContainedItemIds.AddUnique(EditableSkill->BaseItem.InstanceId);
	SlotEntry->SetEquippedSkillId(FGuid());

	if (InSlotIndex == 0)
	{
		QuickSlots.Slot1SkillId.Invalidate();
	}
	else if (InSlotIndex == 1)
	{
		QuickSlots.Slot2SkillId.Invalidate();
	}

	PublishInventoryUpdatedMessage(MainInventory.ContainerId);
	PublishQuickSlotUpdatedMessage(InSlotIndex);
	return true;
}

bool USMInventoryComponent::UnequipSkillFromQuickSlotToMainInventory(
	int32 InSlotIndex,
	int32 InGridX,
	int32 InGridY,
	ESMGridRotation InRotation)
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	if (IsValidQuickSlotIndex(InSlotIndex) == false)
	{
		return false;
	}

	FSMQuickSlotEntry* SlotEntry = FindEditableQuickSlotEntry(InSlotIndex);
	if (SlotEntry == nullptr || SlotEntry->GetEquippedSkillId().IsValid() == false)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(SlotEntry->GetEquippedSkillId());
	if (EditableSkill == nullptr)
	{
		return false;
	}

	if (CanPlaceItem(EditableSkill->BaseItem.InstanceId, MainInventory.ContainerId, InGridX, InGridY, InRotation) == false)
	{
		return false;
	}

	EditableSkill->BaseItem.ParentContainerId = MainInventory.ContainerId;
	EditableSkill->BaseItem.GridX = InGridX;
	EditableSkill->BaseItem.GridY = InGridY;
	EditableSkill->BaseItem.Rotation = InRotation;

	MainInventory.ContainedItemIds.AddUnique(EditableSkill->BaseItem.InstanceId);
	SlotEntry->SetEquippedSkillId(FGuid());

	if (InSlotIndex == 0)
	{
		QuickSlots.Slot1SkillId.Invalidate();
	}
	else if (InSlotIndex == 1)
	{
		QuickSlots.Slot2SkillId.Invalidate();
	}

	PublishInventoryUpdatedMessage(MainInventory.ContainerId);
	PublishQuickSlotUpdatedMessage(InSlotIndex);
	return true;
}

void USMInventoryComponent::SetActiveQuickSlot(int32 InSlotIndex)
{
	if (IsValidQuickSlotIndex(InSlotIndex) == false)
	{
		return;
	}

	if (QuickSlots.ActiveSlotIndex == InSlotIndex)
	{
		return;
	}

	QuickSlots.ActiveSlotIndex = InSlotIndex;
	PublishQuickSlotUpdatedMessage(InSlotIndex);
}

bool USMInventoryComponent::RebuildSkillSummary(const FGuid& InSkillInstanceId)
{
	if (GetOwner() == nullptr || GetOwner()->HasAuthority() == false)
	{
		return false;
	}

	FSMSkillItemInstanceData* EditableSkillData = FindEditableSkill(InSkillInstanceId);
	if (EditableSkillData == nullptr)
	{
		return false;
	}

	FSMCompiledSkillSummary RebuiltSummary;
	if (BuildSkillSummary(InSkillInstanceId, RebuiltSummary) == false)
	{
		return false;
	}

	EditableSkillData->SetCachedSummary(RebuiltSummary);
	return true;
}

bool USMInventoryComponent::GetItemData(const FGuid& InItemInstanceId, FSMItemInstanceData& OutItemData) const
{
	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	if (ItemData == nullptr)
	{
		return false;
	}

	OutItemData = *ItemData;
	return true;
}

bool USMInventoryComponent::GetSkillData(const FGuid& InSkillInstanceId, FSMSkillItemInstanceData& OutSkillData) const
{
	const FSMSkillItemInstanceData* SkillData = FindSkill(InSkillInstanceId);
	if (SkillData == nullptr)
	{
		return false;
	}

	OutSkillData = *SkillData;
	return true;
}

bool USMInventoryComponent::CanDropItem(const FGuid& InItemInstanceId) const
{
	return CanDropItemInternal(InItemInstanceId);
}

bool USMInventoryComponent::GetActiveSkillSummary(FSMCompiledSkillSummary& OutSummary) const
{
	OutSummary.Reset();

	FGuid ActiveSkillId;
	if (const FSMQuickSlotEntry* ActiveSlot = FindQuickSlotEntry(QuickSlots.ActiveSlotIndex))
	{
		ActiveSkillId = ActiveSlot->GetEquippedSkillId();
	}

	if (ActiveSkillId.IsValid() == false)
	{
		ActiveSkillId = QuickSlots.ActiveSlotIndex == 1
			                ? QuickSlots.Slot2SkillId
			                : QuickSlots.Slot1SkillId;
	}

	if (ActiveSkillId.IsValid() == false)
	{
		return false;
	}

	const FSMSkillItemInstanceData* SkillData = FindSkill(ActiveSkillId);
	if (SkillData == nullptr)
	{
		return false;
	}

	OutSummary = SkillData->GetCachedSummary();
	return true;
}

FGameplayTag USMInventoryComponent::GetActiveSkillTag() const
{
	FGuid ActiveSkillId;
	if (const FSMQuickSlotEntry* ActiveSlot = FindQuickSlotEntry(QuickSlots.ActiveSlotIndex))
	{
		ActiveSkillId = ActiveSlot->GetEquippedSkillId();
	}

	if (ActiveSkillId.IsValid() == false)
	{
		ActiveSkillId = QuickSlots.ActiveSlotIndex == 1
			                ? QuickSlots.Slot2SkillId
			                : QuickSlots.Slot1SkillId;
	}

	if (ActiveSkillId.IsValid() == false)
	{
		return FGameplayTag();
	}

	const FSMSkillItemInstanceData* SkillData = FindSkill(ActiveSkillId);
	if (SkillData == nullptr)
	{
		return FGameplayTag();
	}

	const USMItemDefinition* SkillDefinition = ResolveItemDefinition(SkillData->BaseItem);
	if (SkillDefinition == nullptr)
	{
		return FGameplayTag();
	}

	const USMAbilityFragment* AbilityFragment = SkillDefinition->FindFragmentByClass<USMAbilityFragment>();
	if (AbilityFragment == nullptr)
	{
		return FGameplayTag();
	}

	return AbilityFragment->GetAbilityInputTag();
}

bool USMInventoryComponent::GetContainerData(const FGuid& InContainerId, FSMGridContainerState& OutContainerData) const
{
	const FSMGridContainerState* ContainerData = FindContainer(InContainerId);
	if (ContainerData == nullptr)
	{
		return false;
	}

	OutContainerData = *ContainerData;
	return true;
}

void USMInventoryComponent::OnRep_InventoryStateChanged()
{
	if (MainInventory.ContainerId.IsValid())
	{
		PublishInventoryUpdatedMessage(MainInventory.ContainerId);
	}

	for (const FSMGridContainerState& InternalContainer : SkillInternalContainers)
	{
		if (InternalContainer.ContainerId.IsValid())
		{
			PublishInventoryUpdatedMessage(InternalContainer.ContainerId);
		}
	}

	for (const FSMSkillItemInstanceData& SkillEntry : SkillEntries)
	{
		if (SkillEntry.BaseItem.InstanceId.IsValid())
		{
			PublishSkillSummaryUpdatedMessage(SkillEntry.BaseItem.InstanceId);
		}
	}

	PublishQuickSlotUpdatedMessage(0);
	PublishQuickSlotUpdatedMessage(1);
}

bool USMInventoryComponent::HasItem(const FGuid& InItemInstanceId) const
{
	return FindItem(InItemInstanceId) != nullptr || FindSkill(InItemInstanceId) != nullptr;
}

bool USMInventoryComponent::OwnsContainer(const FGuid& InContainerId) const
{
	return FindContainer(InContainerId) != nullptr;
}

const FSMItemInstanceData* USMInventoryComponent::FindItem(const FGuid& InItemInstanceId) const
{
	for (const FSMItemInstanceData& Entry : ItemEntries)
	{
		if (Entry.InstanceId == InItemInstanceId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FSMSkillItemInstanceData* USMInventoryComponent::FindSkill(const FGuid& InSkillInstanceId) const
{
	for (const FSMSkillItemInstanceData& Entry : SkillEntries)
	{
		if (Entry.BaseItem.InstanceId == InSkillInstanceId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FSMGridContainerState* USMInventoryComponent::FindContainer(const FGuid& InContainerId) const
{
	if (MainInventory.ContainerId == InContainerId)
	{
		return &MainInventory;
	}

	for (const FSMGridContainerState& Entry : SkillInternalContainers)
	{
		if (Entry.ContainerId == InContainerId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const FSMQuickSlotEntry* USMInventoryComponent::FindQuickSlotEntry(int32 InSlotIndex) const
{
	for (const FSMQuickSlotEntry& Entry : QuickSlots.Slots)
	{
		if (Entry.GetSlotIndex() == InSlotIndex)
		{
			return &Entry;
		}
	}

	return nullptr;
}

const USMItemDefinition* USMInventoryComponent::ResolveItemDefinition(const FSMItemInstanceData& InItemData) const
{
	if (InItemData.Definition.IsNull())
	{
		return nullptr;
	}

	return InItemData.Definition.LoadSynchronous();
}

FSMItemInstanceData* USMInventoryComponent::FindEditableItem(const FGuid& InItemInstanceId)
{
	for (FSMItemInstanceData& Entry : ItemEntries)
	{
		if (Entry.InstanceId == InItemInstanceId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

FSMSkillItemInstanceData* USMInventoryComponent::FindEditableSkill(const FGuid& InSkillInstanceId)
{
	for (FSMSkillItemInstanceData& Entry : SkillEntries)
	{
		if (Entry.BaseItem.InstanceId == InSkillInstanceId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

FSMGridContainerState* USMInventoryComponent::FindEditableContainer(const FGuid& InContainerId)
{
	if (MainInventory.ContainerId == InContainerId)
	{
		return &MainInventory;
	}

	for (FSMGridContainerState& Entry : SkillInternalContainers)
	{
		if (Entry.ContainerId == InContainerId)
		{
			return &Entry;
		}
	}

	return nullptr;
}

FSMQuickSlotEntry* USMInventoryComponent::FindEditableQuickSlotEntry(int32 InSlotIndex)
{
	for (FSMQuickSlotEntry& Entry : QuickSlots.Slots)
	{
		if (Entry.GetSlotIndex() == InSlotIndex)
		{
			return &Entry;
		}
	}

	return nullptr;
}

void USMInventoryComponent::InitializeMainInventory()
{
	if (MainInventory.ContainerId.IsValid())
	{
		return;
	}

	MainInventory.ContainerId = FGuid::NewGuid();
	MainInventory.ContainerType = ESMContainerType::MainInventory;
	MainInventory.ValidMask = DefaultMainInventoryMask;
	MainInventory.ContainedItemIds.Reset();
}

void USMInventoryComponent::InitializeQuickSlots()
{
	if (QuickSlots.Slots.Num() == 0)
	{
		FSMQuickSlotEntry FirstSlot;
		FirstSlot.SetContainerId(FGuid::NewGuid());
		FirstSlot.SetSlotIndex(0);
		FirstSlot.SetEquippedSkillId(QuickSlots.Slot1SkillId);
		QuickSlots.Slots.Add(FirstSlot);

		FSMQuickSlotEntry SecondSlot;
		SecondSlot.SetContainerId(FGuid::NewGuid());
		SecondSlot.SetSlotIndex(1);
		SecondSlot.SetEquippedSkillId(QuickSlots.Slot2SkillId);
		QuickSlots.Slots.Add(SecondSlot);
	}

	if (FSMQuickSlotEntry* FirstSlot = FindEditableQuickSlotEntry(0))
	{
		QuickSlots.Slot1SkillId = FirstSlot->GetEquippedSkillId();
	}
	else
	{
		QuickSlots.Slot1SkillId.Invalidate();
	}

	if (FSMQuickSlotEntry* SecondSlot = FindEditableQuickSlotEntry(1))
	{
		QuickSlots.Slot2SkillId = SecondSlot->GetEquippedSkillId();
	}
	else
	{
		QuickSlots.Slot2SkillId.Invalidate();
	}

	if (QuickSlots.ActiveSlotIndex < 0 || QuickSlots.ActiveSlotIndex > 1)
	{
		QuickSlots.ActiveSlotIndex = 0;
	}
}

bool USMInventoryComponent::CreateSkillInternalContainer(const FGuid& InSkillInstanceId,
                                                         const FSMGridMaskData& InInternalMask,
                                                         FGuid& OutCreatedContainerId)
{
	if (InSkillInstanceId.IsValid() == false)
	{
		return false;
	}

	if (InInternalMask.IsValidMaskData() == false)
	{
		return false;
	}

	FSMGridContainerState NewContainerState;
	NewContainerState.ContainerId = FGuid::NewGuid();
	NewContainerState.ContainerType = ESMContainerType::SkillInternal;
	NewContainerState.ValidMask = InInternalMask;
	NewContainerState.ContainedItemIds.Reset();

	OutCreatedContainerId = NewContainerState.ContainerId;
	SkillInternalContainers.Add(NewContainerState);
	return true;
}

bool USMInventoryComponent::CanApplyGemToSkillByTags(const USMGemModifierFragment* InGemModifierFragment,
                                                     const USMItemDefinition* InTargetSkillDefinition) const
{
	if (InGemModifierFragment == nullptr || InTargetSkillDefinition == nullptr)
	{
		return false;
	}

	const FGameplayTagContainer& RequiredTargetTags = InGemModifierFragment->RequiredTargetTags;
	const FGameplayTagContainer& BlockedTargetTags = InGemModifierFragment->BlockedTargetTags;

	if (RequiredTargetTags.IsEmpty() && BlockedTargetTags.IsEmpty())
	{
		return true;
	}

	const FGameplayTagContainer& TargetSkillTags = InTargetSkillDefinition->GetItemTags();

	if (RequiredTargetTags.IsEmpty() == false && TargetSkillTags.HasAll(RequiredTargetTags) == false)
	{
		return false;
	}

	if (BlockedTargetTags.IsEmpty() == false && TargetSkillTags.HasAny(BlockedTargetTags))
	{
		return false;
	}

	return true;
}

bool USMInventoryComponent::BuildOccupiedCells(const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY,
                                               ESMGridRotation InRotation, TArray<FIntPoint>& OutOccupiedCells) const
{
	OutOccupiedCells.Reset();

	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	const FSMItemInstanceData* BaseItemData = ItemData;

	if (BaseItemData == nullptr)
	{
		const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
		if (SkillData == nullptr)
		{
			return false;
		}

		BaseItemData = &SkillData->BaseItem;
	}

	const USMItemDefinition* ItemDefinition = ResolveItemDefinition(*BaseItemData);
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
	if (GridShapeFragment == nullptr)
	{
		return false;
	}

	const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
	if (ShapeMask.IsValidMaskData() == false)
	{
		return false;
	}

	for (int32 Y = 0; Y < ShapeMask.Height; ++Y)
	{
		for (int32 X = 0; X < ShapeMask.Width; ++X)
		{
			const int32 MaskIndex = (Y * ShapeMask.Width) + X;
			if (ShapeMask.BitMask.IsValidIndex(MaskIndex) == false)
			{
				continue;
			}

			if (ShapeMask.BitMask[MaskIndex] != TEXT('1'))
			{
				continue;
			}

			int32 RotatedX = X;
			int32 RotatedY = Y;

			switch (InRotation)
			{
			case ESMGridRotation::Rot0:
				RotatedX = X;
				RotatedY = Y;
				break;

			case ESMGridRotation::Rot90:
				RotatedX = ShapeMask.Height - 1 - Y;
				RotatedY = X;
				break;

			case ESMGridRotation::Rot180:
				RotatedX = ShapeMask.Width - 1 - X;
				RotatedY = ShapeMask.Height - 1 - Y;
				break;

			case ESMGridRotation::Rot270:
				RotatedX = Y;
				RotatedY = ShapeMask.Width - 1 - X;
				break;

			default:
				return false;
			}

			OutOccupiedCells.Add(FIntPoint(InGridX + RotatedX, InGridY + RotatedY));
		}
	}

	return OutOccupiedCells.Num() > 0;
}

void USMInventoryComponent::AddOccupiedBits(TBitArray<>& InOutOccupiedBits, const FSMGridContainerState& InTargetContainer,
                                            const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY,
                                            ESMGridRotation InRotation) const
{
	TArray<FIntPoint> OccupiedCells;
	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, InRotation, OccupiedCells) == false)
	{
		return;
	}

	for (const FIntPoint& Cell : OccupiedCells)
	{
		if (Cell.X < 0 || Cell.Y < 0 || Cell.X >= InTargetContainer.ValidMask.Width || Cell.Y >= InTargetContainer.ValidMask.Height)
		{
			continue;
		}

		const int32 CellIndex = (Cell.Y * InTargetContainer.ValidMask.Width) + Cell.X;
		if (InOutOccupiedBits.IsValidIndex(CellIndex))
		{
			InOutOccupiedBits[CellIndex] = true;
		}
	}
}

bool USMInventoryComponent::HasPlacementConflict(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId,
                                                 int32 InGridX, int32 InGridY, ESMGridRotation InRotation) const
{
	TArray<FIntPoint> TargetCells;
	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, InRotation, TargetCells) == false)
	{
		return true;
	}

	const FSMGridContainerState* TargetContainer = FindContainer(InTargetContainerId);
	if (TargetContainer == nullptr)
	{
		return true;
	}

	TBitArray<> OccupiedBits;
	OccupiedBits.Init(false, TargetContainer->ValidMask.Width * TargetContainer->ValidMask.Height);

	for (const FSMItemInstanceData& Entry : ItemEntries)
	{
		if (Entry.InstanceId == InItemInstanceId)
		{
			continue;
		}

		if (Entry.ParentContainerId != InTargetContainerId)
		{
			continue;
		}

		AddOccupiedBits(OccupiedBits, *TargetContainer, Entry.InstanceId, Entry.GridX, Entry.GridY, Entry.Rotation);
	}

	for (const FSMSkillItemInstanceData& Entry : SkillEntries)
	{
		if (Entry.BaseItem.InstanceId == InItemInstanceId)
		{
			continue;
		}

		if (Entry.BaseItem.ParentContainerId != InTargetContainerId)
		{
			continue;
		}

		AddOccupiedBits(OccupiedBits, *TargetContainer, Entry.BaseItem.InstanceId, Entry.BaseItem.GridX, Entry.BaseItem.GridY, Entry.BaseItem.Rotation);
	}

	for (const FIntPoint& TargetCell : TargetCells)
	{
		if (TargetCell.X < 0 || TargetCell.Y < 0 || TargetCell.X >= TargetContainer->ValidMask.Width || TargetCell.Y >= TargetContainer->ValidMask.Height)
		{
			return true;
		}

		const int32 CellIndex = (TargetCell.Y * TargetContainer->ValidMask.Width) + TargetCell.X;
		if (OccupiedBits.IsValidIndex(CellIndex) && OccupiedBits[CellIndex])
		{
			return true;
		}
	}

	return false;
}

bool USMInventoryComponent::IsSameNamedSkill(const FGuid& InAItemInstanceId, const FGuid& InBItemInstanceId) const
{
	const FSMSkillItemInstanceData* SkillA = FindSkill(InAItemInstanceId);
	const FSMSkillItemInstanceData* SkillB = FindSkill(InBItemInstanceId);

	if (SkillA == nullptr || SkillB == nullptr)
	{
		return false;
	}

	const USMItemDefinition* SkillADefinition = ResolveItemDefinition(SkillA->BaseItem);
	const USMItemDefinition* SkillBDefinition = ResolveItemDefinition(SkillB->BaseItem);

	if (SkillADefinition == nullptr || SkillBDefinition == nullptr)
	{
		return false;
	}

	if (SkillADefinition->GetInternalName().IsNone() == false &&
		SkillBDefinition->GetInternalName().IsNone() == false)
	{
		return SkillADefinition->GetInternalName() == SkillBDefinition->GetInternalName();
	}

	return SkillA->BaseItem.Definition == SkillB->BaseItem.Definition;
}

bool USMInventoryComponent::IsSkillActuallyEmpty(const FGuid& InSkillInstanceId) const
{
	const FSMSkillItemInstanceData* SkillData = FindSkill(InSkillInstanceId);
	if (SkillData == nullptr)
	{
		return false;
	}

	if (SkillData->EmbeddedItemIds.Num() > 0)
	{
		return false;
	}

	const FSMGridContainerState* InternalContainer = FindContainer(SkillData->InternalContainerId);
	if (InternalContainer == nullptr)
	{
		return false;
	}

	return InternalContainer->ContainedItemIds.Num() == 0;
}

bool USMInventoryComponent::CanDropItemInternal(const FGuid& InItemInstanceId) const
{
	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	const FSMItemInstanceData* BaseItemData = ItemData;

	if (BaseItemData == nullptr)
	{
		const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
		if (SkillData == nullptr)
		{
			return false;
		}

		BaseItemData = &SkillData->BaseItem;
	}

	if (BaseItemData->bLocked)
	{
		return false;
	}

	const USMItemDefinition* ItemDefinition = ResolveItemDefinition(*BaseItemData);
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const USMDropRuleFragment* DropRuleFragment = ItemDefinition->FindFragmentByClass<USMDropRuleFragment>();
	if (DropRuleFragment != nullptr && DropRuleFragment->CanDrop() == false)
	{
		return false;
	}

	const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
	if (SkillData != nullptr && IsSkillActuallyEmpty(InItemInstanceId) == false)
	{
		if (DropRuleFragment != nullptr && DropRuleFragment->CanDropWithEmbeddedItems() == false)
		{
			return false;
		}
	}

	return true;
}

bool USMInventoryComponent::BuildDropPayload(const FGuid& InItemInstanceId, FSMItemDropPayload& OutPayload) const
{
	OutPayload = FSMItemDropPayload();

	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	if (ItemData != nullptr)
	{
		OutPayload.SetInstanceId(ItemData->InstanceId);
		OutPayload.ItemType = ItemData->ItemType;
		OutPayload.SetDefinition(ItemData->Definition);
		OutPayload.SetRotation(ItemData->Rotation);
		OutPayload.SetLocked(ItemData->bLocked);
		OutPayload.SetNestedItemSnapshots(TArray<FSMNestedItemDropSnapshot>());
		return OutPayload.IsValidPayload();
	}

	const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
	if (SkillData == nullptr)
	{
		return false;
	}

	OutPayload.SetInstanceId(SkillData->BaseItem.InstanceId);
	OutPayload.ItemType = SkillData->BaseItem.ItemType;
	OutPayload.SetDefinition(SkillData->BaseItem.Definition);
	OutPayload.SetRotation(SkillData->BaseItem.Rotation);
	OutPayload.SetLocked(SkillData->BaseItem.bLocked);

	TArray<FSMNestedItemDropSnapshot> NestedSnapshots;
	CollectNestedDropSnapshots(SkillData->BaseItem.InstanceId, NestedSnapshots);
	OutPayload.SetNestedItemSnapshots(NestedSnapshots);

	return OutPayload.IsValidPayload();
}

ASMBaseItemDropActor* USMInventoryComponent::SpawnDropActorFromPayload(const FSMItemDropPayload& InPayload,
                                                                       const FTransform& InSpawnTransform)
{
	if (GetOwner() == nullptr)
	{
		return nullptr;
	}

	if (GetOwner()->HasAuthority() == false)
	{
		return nullptr;
	}

	if (InPayload.IsValidPayload() == false)
	{
		return nullptr;
	}

	if (DefaultDropActorClass == nullptr)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASMBaseItemDropActor* SpawnedDropActor = World->SpawnActor<ASMBaseItemDropActor>(
		DefaultDropActorClass,
		InSpawnTransform,
		SpawnParams);

	if (SpawnedDropActor == nullptr)
	{
		return nullptr;
	}

	SpawnedDropActor->InitializeFromPayload(InPayload);
	return SpawnedDropActor;
}

bool USMInventoryComponent::BuildSkillSummary(const FGuid& InSkillInstanceId, FSMCompiledSkillSummary& OutSummary) const
{
	OutSummary.Reset();

	const FSMSkillItemInstanceData* SkillData = FindSkill(InSkillInstanceId);
	if (SkillData == nullptr)
	{
		return false;
	}

	const USMItemDefinition* SkillDefinition = ResolveItemDefinition(SkillData->BaseItem);
	if (SkillDefinition == nullptr)
	{
		return false;
	}

	const FSMGridContainerState* InternalContainer = FindContainer(SkillData->InternalContainerId);
	if (InternalContainer == nullptr)
	{
		return false;
	}

	int32 CurrentLevel = 1;
	int32 MaxLevel = TNumericLimits<int32>::Max();
	if (const USMSkillProgressionFragment* SkillProgressionFragment =
		SkillDefinition->FindFragmentByClass<USMSkillProgressionFragment>())
	{
		CurrentLevel = FMath::Max(1, SkillProgressionFragment->GetBaseLevel());
		MaxLevel = FMath::Max(CurrentLevel, SkillProgressionFragment->GetMaxLevel());
	}

	const USMAbilityFragment* AbilityFragment = SkillDefinition->FindFragmentByClass<USMAbilityFragment>();
	if (AbilityFragment == nullptr)
	{
		return false;
	}

	const FGameplayTag ResolvedSkillTag = AbilityFragment->GetAbilityInputTag();

	TArray<const FSMItemInstanceData*> EmbeddedGems;
	EmbeddedGems.Reserve(InternalContainer->ContainedItemIds.Num());

	for (const FGuid& EmbeddedItemInstanceId : InternalContainer->ContainedItemIds)
	{
		if (const FSMSkillItemInstanceData* EmbeddedSkillData = FindSkill(EmbeddedItemInstanceId))
		{
			OutSummary.EmbeddedSkillIds.Add(EmbeddedSkillData->BaseItem.InstanceId);
			if (IsSameNamedSkill(InSkillInstanceId, EmbeddedItemInstanceId))
			{
				++CurrentLevel;
			}
			continue;
		}

		const FSMItemInstanceData* EmbeddedItemData = FindItem(EmbeddedItemInstanceId);
		if (EmbeddedItemData != nullptr && EmbeddedItemData->ItemType == ESMItemType::Gem)
		{
			EmbeddedGems.Add(EmbeddedItemData);
		}
	}

	CurrentLevel = FMath::Clamp(CurrentLevel, 1, MaxLevel);
	if (ResolvedSkillTag.IsValid() == false)
	{
		return false;
	}

	USMSyncDataManager* SyncDataManager = USMSyncDataManager::Get(this);
	if (SyncDataManager == nullptr)
	{
		return false;
	}

	const FSMSkillData SkillRuntimeData = SyncDataManager->GetSkillData(ResolvedSkillTag);
	if (SkillRuntimeData.SkillTag != ResolvedSkillTag)
	{
		return false;
	}

	const FSMSkillLevelData* MatchedLevelData = SkillRuntimeData.LevelData.FindByPredicate(
		[CurrentLevel](const FSMSkillLevelData& LevelData)
		{
			return LevelData.Level == CurrentLevel;
		});
	if (MatchedLevelData == nullptr)
	{
		return false;
	}

	float FinalDamage = SkillRuntimeData.BaseDamage + MatchedLevelData->BaseDamage;
	float FinalRangeOrArea = SkillRuntimeData.RangeCm + MatchedLevelData->RangeCm;
	float FinalCooldown = SkillRuntimeData.Cooldown + MatchedLevelData->Cooldown;
	FGameplayTagContainer BehaviorTags = MatchedLevelData->BehaviorTags;

	for (const FSMItemInstanceData* EmbeddedGemData : EmbeddedGems)
	{
		const USMItemDefinition* EmbeddedGemDefinition = ResolveItemDefinition(*EmbeddedGemData);
		if (EmbeddedGemDefinition == nullptr)
		{
			continue;
		}

		const USMGemModifierFragment* GemModifierFragment =
			EmbeddedGemDefinition->FindFragmentByClass<USMGemModifierFragment>();
		if (GemModifierFragment == nullptr)
		{
			continue;
		}

		OutSummary.EmbeddedGemIds.Add(EmbeddedGemData->InstanceId);
		BehaviorTags.AppendTags(GemModifierFragment->GetGrantedBehaviorTags());

		switch (GemModifierFragment->GetModifierType())
		{
		case ESMGemModifierType::Effect:
			FinalDamage += static_cast<float>(GemModifierFragment->GetModifierValue());
			break;
		case ESMGemModifierType::RangeOrArea:
			FinalRangeOrArea += static_cast<float>(GemModifierFragment->GetModifierValue());
			break;
		case ESMGemModifierType::Cooldown:
			FinalCooldown += static_cast<float>(GemModifierFragment->GetModifierValue());
			break;
		case ESMGemModifierType::None:
		default:
			break;
		}
	}

	OutSummary.SetCurrentLevel(CurrentLevel);
	OutSummary.SetFinalDamage(FinalDamage);
	OutSummary.SetFinalRangeOrArea(FinalRangeOrArea);
	OutSummary.SetFinalCooldown(FMath::Max(0.0f, FinalCooldown));
	OutSummary.SetBehaviorTags(BehaviorTags);
	return true;
}

bool USMInventoryComponent::IsValidQuickSlotIndex(int32 InSlotIndex) const
{
	return FindQuickSlotEntry(InSlotIndex) != nullptr;
}

void USMInventoryComponent::PublishInventoryUpdatedMessage(const FGuid& InContainerId) const
{
	APlayerState* OwningPlayerState = Cast<APlayerState>(GetOwner());
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	const FSMGridContainerState* ContainerState = FindContainer(InContainerId);
	if (ContainerState == nullptr)
	{
		return;
	}

	FSMInventoryUpdatedMessage Message;
	Message.SetOwningPlayerState(OwningPlayerState);
	Message.SetContainerId(InContainerId);

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);

	if (ContainerState->ContainerType == ESMContainerType::SkillInternal)
	{
		MessageSubsystem.BroadcastMessage(SMMessageTag::Inventory_SkillContainerUpdated, Message);
		return;
	}

	MessageSubsystem.BroadcastMessage(SMMessageTag::Inventory_MainContainerUpdated, Message);
}

void USMInventoryComponent::PublishSkillSummaryUpdatedMessage(const FGuid& InSkillInstanceId) const
{
	APlayerState* OwningPlayerState = Cast<APlayerState>(GetOwner());
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	FSMSkillSummaryUpdatedMessage Message;
	Message.SetOwningPlayerState(OwningPlayerState);
	Message.SetSkillInstanceId(InSkillInstanceId);

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(SMMessageTag::Inventory_SkillSummaryUpdated, Message);
}

void USMInventoryComponent::PublishQuickSlotUpdatedMessage(int32 InSlotIndex) const
{
	APlayerState* OwningPlayerState = Cast<APlayerState>(GetOwner());
	if (OwningPlayerState == nullptr)
	{
		return;
	}

	FSMQuickSlotUpdatedMessage Message;
	Message.SetOwningPlayerState(OwningPlayerState);
	Message.SetSlotIndex(InSlotIndex);

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(SMMessageTag::Inventory_QuickSlotUpdated, Message);
}

void USMInventoryComponent::CollectNestedDropSnapshots(const FGuid& InParentSkillInstanceId,
                                                       TArray<FSMNestedItemDropSnapshot>& OutSnapshots) const
{
	const FSMSkillItemInstanceData* ParentSkillData = FindSkill(InParentSkillInstanceId);
	if (ParentSkillData == nullptr)
	{
		return;
	}

	const FSMGridContainerState* InternalContainer = FindContainer(ParentSkillData->InternalContainerId);
	if (InternalContainer == nullptr)
	{
		return;
	}

	for (const FGuid& ChildItemInstanceId : InternalContainer->ContainedItemIds)
	{
		const FSMItemInstanceData* ChildItemData = FindItem(ChildItemInstanceId);
		if (ChildItemData != nullptr)
		{
			FSMNestedItemDropSnapshot Snapshot;
			Snapshot.SetInstanceId(ChildItemData->InstanceId);
			Snapshot.SetParentSkillInstanceId(InParentSkillInstanceId);
			Snapshot.ItemType = ChildItemData->ItemType;
			Snapshot.SetDefinition(ChildItemData->Definition);
			Snapshot.SetGridX(ChildItemData->GridX);
			Snapshot.SetGridY(ChildItemData->GridY);
			Snapshot.SetRotation(ChildItemData->Rotation);
			Snapshot.SetLocked(ChildItemData->bLocked);

			OutSnapshots.Add(Snapshot);
			continue;
		}

		const FSMSkillItemInstanceData* ChildSkillData = FindSkill(ChildItemInstanceId);
		if (ChildSkillData != nullptr)
		{
			FSMNestedItemDropSnapshot Snapshot;
			Snapshot.SetInstanceId(ChildSkillData->BaseItem.InstanceId);
			Snapshot.SetParentSkillInstanceId(InParentSkillInstanceId);
			Snapshot.ItemType = ChildSkillData->BaseItem.ItemType;
			Snapshot.SetDefinition(ChildSkillData->BaseItem.Definition);
			Snapshot.SetGridX(ChildSkillData->BaseItem.GridX);
			Snapshot.SetGridY(ChildSkillData->BaseItem.GridY);
			Snapshot.SetRotation(ChildSkillData->BaseItem.Rotation);
			Snapshot.SetLocked(ChildSkillData->BaseItem.bLocked);

			OutSnapshots.Add(Snapshot);
			CollectNestedDropSnapshots(ChildSkillData->BaseItem.InstanceId, OutSnapshots);
		}
	}
}

FGuid USMInventoryComponent::AddNestedSnapshotToContainer(const FSMNestedItemDropSnapshot& InSnapshot,
                                                          const FGuid& InTargetContainerId)
{
	const USMItemDefinition* ItemDefinition = InSnapshot.GetDefinition().LoadSynchronous();
	if (ItemDefinition == nullptr)
	{
		return FGuid();
	}

	if (InSnapshot.ItemType == ESMItemType::Skill)
	{
		const USMInternalInventoryFragment* InternalInventoryFragment =
			ItemDefinition->FindFragmentByClass<USMInternalInventoryFragment>();
		if (InternalInventoryFragment == nullptr)
		{
			return FGuid();
		}

		FSMSkillItemInstanceData NewSkillEntry;
		NewSkillEntry.BaseItem.InstanceId = InSnapshot.GetInstanceId();
		NewSkillEntry.BaseItem.ItemType = ESMItemType::Skill;
		NewSkillEntry.BaseItem.Definition = InSnapshot.GetDefinition();
		NewSkillEntry.BaseItem.ParentContainerId = InTargetContainerId;
		NewSkillEntry.BaseItem.GridX = InSnapshot.GetGridX();
		NewSkillEntry.BaseItem.GridY = InSnapshot.GetGridY();
		NewSkillEntry.BaseItem.Rotation = InSnapshot.GetRotation();
		NewSkillEntry.BaseItem.bLocked = InSnapshot.IsLocked();

		FGuid InternalContainerId;
		if (CreateSkillInternalContainer(
			NewSkillEntry.BaseItem.InstanceId,
			InternalInventoryFragment->GetInternalMask(),
			InternalContainerId) == false)
		{
			return FGuid();
		}

		NewSkillEntry.InternalContainerId = InternalContainerId;
		NewSkillEntry.EmbeddedItemIds.Reset();

		SkillEntries.Add(NewSkillEntry);

		if (CanPlaceItem(
			NewSkillEntry.BaseItem.InstanceId,
			InTargetContainerId,
			InSnapshot.GetGridX(),
			InSnapshot.GetGridY(),
			InSnapshot.GetRotation()) == false)
		{
			SkillEntries.RemoveAll(
				[&](const FSMSkillItemInstanceData& Entry)
				{
					return Entry.BaseItem.InstanceId == NewSkillEntry.BaseItem.InstanceId;
				});

			SkillInternalContainers.RemoveAll(
				[&](const FSMGridContainerState& ContainerState)
				{
					return ContainerState.ContainerId == InternalContainerId;
				});

			return FGuid();
		}

		FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(NewSkillEntry.BaseItem.InstanceId);
		if (EditableSkill == nullptr)
		{
			SkillEntries.RemoveAll(
				[&](const FSMSkillItemInstanceData& Entry)
				{
					return Entry.BaseItem.InstanceId == NewSkillEntry.BaseItem.InstanceId;
				});

			SkillInternalContainers.RemoveAll(
				[&](const FSMGridContainerState& ContainerState)
				{
					return ContainerState.ContainerId == InternalContainerId;
				});

			return FGuid();
		}

		EditableSkill->BaseItem.GridX = InSnapshot.GetGridX();
		EditableSkill->BaseItem.GridY = InSnapshot.GetGridY();

		if (FSMGridContainerState* TargetContainer = FindEditableContainer(InTargetContainerId))
		{
			TargetContainer->ContainedItemIds.Add(EditableSkill->BaseItem.InstanceId);
		}

		PublishInventoryUpdatedMessage(InTargetContainerId);
		return EditableSkill->BaseItem.InstanceId;
	}

	FSMItemInstanceData NewItemEntry;
	NewItemEntry.InstanceId = InSnapshot.GetInstanceId();
	NewItemEntry.ItemType = InSnapshot.ItemType;
	NewItemEntry.Definition = InSnapshot.GetDefinition();
	NewItemEntry.ParentContainerId = InTargetContainerId;
	NewItemEntry.GridX = InSnapshot.GetGridX();
	NewItemEntry.GridY = InSnapshot.GetGridY();
	NewItemEntry.Rotation = InSnapshot.GetRotation();
	NewItemEntry.bLocked = InSnapshot.IsLocked();

	ItemEntries.Add(NewItemEntry);

	if (CanPlaceItem(
		NewItemEntry.InstanceId,
		InTargetContainerId,
		InSnapshot.GetGridX(),
		InSnapshot.GetGridY(),
		InSnapshot.GetRotation()) == false)
	{
		ItemEntries.RemoveAll(
			[&](const FSMItemInstanceData& Entry)
			{
				return Entry.InstanceId == NewItemEntry.InstanceId;
			});

		return FGuid();
	}

	FSMItemInstanceData* EditableItem = FindEditableItem(NewItemEntry.InstanceId);
	if (EditableItem == nullptr)
	{
		ItemEntries.RemoveAll(
			[&](const FSMItemInstanceData& Entry)
			{
				return Entry.InstanceId == NewItemEntry.InstanceId;
			});

		return FGuid();
	}

	EditableItem->GridX = InSnapshot.GetGridX();
	EditableItem->GridY = InSnapshot.GetGridY();

	if (FSMGridContainerState* TargetContainer = FindEditableContainer(InTargetContainerId))
	{
		TargetContainer->ContainedItemIds.Add(EditableItem->InstanceId);
	}

	PublishInventoryUpdatedMessage(InTargetContainerId);
	return EditableItem->InstanceId;
}

bool USMInventoryComponent::RestoreNestedPayloads(const FSMItemDropPayload& InDropPayload,
                                                  const FGuid& InParentSkillInstanceId)
{
	FSMSkillItemInstanceData* ParentSkillData = FindEditableSkill(InParentSkillInstanceId);
	if (ParentSkillData == nullptr)
	{
		return false;
	}

	for (const FSMNestedItemDropSnapshot& Snapshot : InDropPayload.GetNestedItemSnapshots())
	{
		if (Snapshot.GetParentSkillInstanceId() != InParentSkillInstanceId)
		{
			continue;
		}

		const FGuid AddedItemInstanceId = AddNestedSnapshotToContainer(Snapshot, ParentSkillData->InternalContainerId);
		if (AddedItemInstanceId.IsValid() == false)
		{
			return false;
		}

		ParentSkillData->EmbeddedItemIds.AddUnique(AddedItemInstanceId);

		if (Snapshot.ItemType == ESMItemType::Skill)
		{
			if (RestoreNestedPayloads(InDropPayload, Snapshot.GetInstanceId()) == false)
			{
				return false;
			}
		}
	}

	return RebuildSkillSummary(InParentSkillInstanceId);
}

bool USMInventoryComponent::RemoveItemInternal(const FGuid& InItemInstanceId, bool bPublishInventoryMessage)
{
	FSMItemInstanceData* EditableItem = FindEditableItem(InItemInstanceId);
	if (EditableItem != nullptr)
	{
		const FGuid ParentContainerId = EditableItem->ParentContainerId;

		if (FSMGridContainerState* ParentContainer = FindEditableContainer(ParentContainerId))
		{
			ParentContainer->ContainedItemIds.Remove(InItemInstanceId);
		}

		for (FSMSkillItemInstanceData& SkillEntry : SkillEntries)
		{
			if (SkillEntry.InternalContainerId == ParentContainerId)
			{
				SkillEntry.EmbeddedItemIds.Remove(InItemInstanceId);
				if (bPublishInventoryMessage)
				{
					RebuildSkillSummary(SkillEntry.BaseItem.InstanceId);
				}
				break;
			}
		}

		ItemEntries.RemoveAll(
			[&](const FSMItemInstanceData& Entry)
			{
				return Entry.InstanceId == InItemInstanceId;
			});

		if (bPublishInventoryMessage)
		{
			PublishInventoryUpdatedMessage(ParentContainerId);
		}

		return true;
	}

	FSMSkillItemInstanceData* EditableSkill = FindEditableSkill(InItemInstanceId);
	if (EditableSkill != nullptr)
	{
		const FGuid ParentContainerId = EditableSkill->BaseItem.ParentContainerId;
		const FGuid InternalContainerId = EditableSkill->InternalContainerId;
		FGuid ParentOwningSkillId;

		TArray<FGuid> ChildItemIds;
		if (const FSMGridContainerState* InternalContainer = FindContainer(InternalContainerId))
		{
			ChildItemIds = InternalContainer->ContainedItemIds;
		}

		for (const FGuid& ChildItemInstanceId : ChildItemIds)
		{
			if (RemoveItemInternal(ChildItemInstanceId, false) == false)
			{
				return false;
			}
		}

		if (FSMGridContainerState* ParentContainer = FindEditableContainer(ParentContainerId))
		{
			ParentContainer->ContainedItemIds.Remove(InItemInstanceId);
		}

		for (FSMSkillItemInstanceData& SkillEntry : SkillEntries)
		{
			if (SkillEntry.InternalContainerId == ParentContainerId)
			{
				SkillEntry.EmbeddedItemIds.Remove(InItemInstanceId);
				ParentOwningSkillId = SkillEntry.BaseItem.InstanceId;
				break;
			}
		}

		for (FSMQuickSlotEntry& SlotEntry : QuickSlots.Slots)
		{
			if (SlotEntry.GetEquippedSkillId() != InItemInstanceId)
			{
				continue;
			}

			SlotEntry.SetEquippedSkillId(FGuid());

			if (SlotEntry.GetSlotIndex() == 0)
			{
				QuickSlots.Slot1SkillId.Invalidate();
			}
			else if (SlotEntry.GetSlotIndex() == 1)
			{
				QuickSlots.Slot2SkillId.Invalidate();
			}

			PublishQuickSlotUpdatedMessage(SlotEntry.GetSlotIndex());
		}

		SkillEntries.RemoveAll(
			[&](const FSMSkillItemInstanceData& Entry)
			{
				return Entry.BaseItem.InstanceId == InItemInstanceId;
			});

		SkillInternalContainers.RemoveAll(
			[&](const FSMGridContainerState& ContainerState)
			{
				return ContainerState.ContainerId == InternalContainerId;
			});

		if (bPublishInventoryMessage && ParentOwningSkillId.IsValid())
		{
			RebuildSkillSummary(ParentOwningSkillId);
		}

		if (bPublishInventoryMessage)
		{
			PublishInventoryUpdatedMessage(ParentContainerId);
		}

		return true;
	}

	return false;
}
