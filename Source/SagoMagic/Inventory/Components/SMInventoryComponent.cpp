#include "Inventory/Components/SMInventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Engine/World.h"

#include "Inventory/Core/SMInventoryMessageTypes.h"

#include "Inventory/Items/Definitions/SMItemDefinition.h"

#include "Inventory/Items/Fragments/SMGridShapeFragment.h"
#include "Inventory/Items/Fragments/SMGemModifierFragment.h"
#include "Inventory/Items/Fragments/SMInternalInventoryFragment.h"
#include "Inventory/Items/Fragments/SMSkillProgressionFragment.h"
#include "Inventory/Items/Fragments/SMDropRuleFragment.h"
#include "Inventory/Items/Fragments/SMWorldVisualFragment.h"

#include "GameplayTags/Message/SMMessageTag.h"

#include "Inventory/World/SMBaseItemDropActor.h"

USMInventoryComponent::USMInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
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
	}
}

void USMInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USMInventoryComponent, MainInventory);
	DOREPLIFETIME(USMInventoryComponent, QuickSlots);
	DOREPLIFETIME(USMInventoryComponent, ItemEntries);
	DOREPLIFETIME(USMInventoryComponent, SkillEntries);
	DOREPLIFETIME(USMInventoryComponent, SkillInternalContainers);
}

FGuid USMInventoryComponent::AddItemFromDefinition(const TSoftObjectPtr<USMItemDefinition>& InItemDefinition)
{
	/** TODO: 아이템 생성 및 배치 처리 */
	return FGuid();
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

	if (CanPlaceItem(InItemInstanceId, InTargetContainerId, InGridX, InGridY, InRotation) == false)
	{
		return false;
	}

	FSMGridContainerState* PreviousContainer = FindEditableContainer(PreviousContainerId);
	if (PreviousContainer != nullptr)
	{
		PreviousContainer->ContainedItemIds.Remove(InItemInstanceId);
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

	for (int32 Y = 0; Y < TargetContainer->ValidMask.Height; ++Y)
	{
		for (int32 X = 0; X < TargetContainer->ValidMask.Width; ++X)
		{
			if (CanPlaceItem(InItemInstanceId, InTargetContainerId, X, Y, RotationToUse))
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
	/** TODO: 장착 가능 여부 검사 */
	/** TODO: 장착 처리 */
	/** TODO: 스킬 요약 재계산 */
	return false;
}

bool USMInventoryComponent::AttachSkillToSkill(const FGuid& InMaterialSkillInstanceId,
                                               const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY)
{
	/** TODO: 동일 이름 / 빈 스킬 여부 검사 */
	/** TODO: 장착 처리 */
	/** TODO: 스킬 요약 재계산 */
	return false;
}

bool USMInventoryComponent::DetachEmbeddedItem(const FGuid& InEmbeddedItemInstanceId)
{
	/** TODO: 부모 스킬 탐색 */
	/** TODO: 해제 처리 */
	/** TODO: 스킬 요약 재계산 */
	return false;
}

bool USMInventoryComponent::EquipSkillToQuickSlot(const FGuid& InSkillInstanceId, int32 InSlotIndex)
{
	/** TODO: 슬롯 유효성 검사 및 장착 처리 */
	return false;
}

bool USMInventoryComponent::UnequipSkillFromQuickSlot(int32 InSlotIndex)
{
	/** TODO: 슬롯 유효성 검사 및 해제 처리 */
	return false;
}

void USMInventoryComponent::SetActiveQuickSlot(int32 InSlotIndex)
{
	/** TODO: 활성 슬롯 갱신 처리 */
}

bool USMInventoryComponent::RebuildSkillSummary(const FGuid& InSkillInstanceId)
{
	/** TODO: BuildSkillSummary 호출 */
	/** TODO: CachedSummary 반영 */
	/** TODO: 메시지 발행 */
	return false;
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

bool USMInventoryComponent::GetSkillSummary(const FGuid& InSkillInstanceId, FSMCompiledSkillSummary& OutSummary) const
{
	/** TODO: 요약 캐시 조회 처리 */
	return false;
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
	/** TODO: UI 갱신용 메시지 발행 */
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
	/** TODO: RequiredTargetTags / BlockedTargetTags 검사 */
	return false;
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

bool USMInventoryComponent::HasPlacementConflict(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId,
                                                 int32 InGridX, int32 InGridY, ESMGridRotation InRotation) const
{
	TArray<FIntPoint> TargetCells;
	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, InRotation, TargetCells) == false)
	{
		return true;
	}

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

		TArray<FIntPoint> ExistingCells;
		if (BuildOccupiedCells(Entry.InstanceId, Entry.GridX, Entry.GridY, Entry.Rotation, ExistingCells) == false)
		{
			continue;
		}

		for (const FIntPoint& TargetCell : TargetCells)
		{
			if (ExistingCells.Contains(TargetCell))
			{
				return true;
			}
		}
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

		TArray<FIntPoint> ExistingCells;
		if (BuildOccupiedCells(Entry.BaseItem.InstanceId, Entry.BaseItem.GridX, Entry.BaseItem.GridY,
		                       Entry.BaseItem.Rotation, ExistingCells) == false)
		{
			continue;
		}

		for (const FIntPoint& TargetCell : TargetCells)
		{
			if (ExistingCells.Contains(TargetCell))
			{
				return true;
			}
		}
	}

	return false;
}

bool USMInventoryComponent::IsSameNamedSkill(const FGuid& InAItemInstanceId, const FGuid& InBItemInstanceId) const
{
	/** TODO: 정의 에셋 내부 이름 비교 */
	return false;
}

bool USMInventoryComponent::IsSkillActuallyEmpty(const FGuid& InSkillInstanceId) const
{
	const FSMSkillItemInstanceData* SkillData = FindSkill(InSkillInstanceId);
	if (SkillData == nullptr)
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
	/** TODO: 장착 젬 / 보조 스킬 기반 총합 계산 */
	return false;
}

bool USMInventoryComponent::IsValidQuickSlotIndex(int32 InSlotIndex) const
{
	/** TODO: 1~2 슬롯 범위 검사 */
	return false;
}

void USMInventoryComponent::PublishInventoryUpdatedMessage(const FGuid& InContainerId) const
{
	/** TODO: Gameplay Message Subsystem 브로드캐스트 */
}

void USMInventoryComponent::PublishSkillSummaryUpdatedMessage(const FGuid& InSkillInstanceId) const
{
	/** TODO: Gameplay Message Subsystem 브로드캐스트 */
}

void USMInventoryComponent::PublishQuickSlotUpdatedMessage(int32 InSlotIndex) const
{
	/** TODO: Gameplay Message Subsystem 브로드캐스트 */
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

	return true;
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
				break;
			}
		}

		if (QuickSlots.Slot1SkillId == InItemInstanceId)
		{
			QuickSlots.Slot1SkillId = FGuid();
			PublishQuickSlotUpdatedMessage(0);
		}

		if (QuickSlots.Slot2SkillId == InItemInstanceId)
		{
			QuickSlots.Slot2SkillId = FGuid();
			PublishQuickSlotUpdatedMessage(1);
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

		if (bPublishInventoryMessage)
		{
			PublishInventoryUpdatedMessage(ParentContainerId);
		}

		return true;
	}

	return false;
}
