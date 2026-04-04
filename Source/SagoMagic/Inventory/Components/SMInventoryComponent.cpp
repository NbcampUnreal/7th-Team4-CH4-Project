#include "Inventory/Components/SMInventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/GameplayMessageSubsystem.h"

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

		/** 새로운 스킬 엔트리 생성 */
		FSMSkillItemInstanceData NewSkillEntry;
		NewSkillEntry.BaseItem.InstanceId = InDropPayload.InstanceId;
		NewSkillEntry.BaseItem.ItemType = ESMItemType::Skill;
		NewSkillEntry.BaseItem.Definition = InDropPayload.GetDefinition();
		NewSkillEntry.BaseItem.ParentContainerId = MainInventory.ContainerId;
		NewSkillEntry.BaseItem.GridX = 0;
		NewSkillEntry.BaseItem.GridY = 0;
		NewSkillEntry.BaseItem.Rotation = InDropPayload.GetRotation();
		NewSkillEntry.BaseItem.bLocked = false;

		/** 내부 컨테이너 복원 */
		FGuid InternalContainerId;
		if (CreateSkillInternalContainer(
			NewSkillEntry.BaseItem.InstanceId, InternalInventoryFragment->GetInternalMask(),
			InternalContainerId) == false)
		{
			return FGuid();
		}

		NewSkillEntry.InternalContainerId = InternalContainerId;
		SkillEntries.Add(NewSkillEntry);

		int32 FoundGridX = 0;
		int32 FoundGridY = 0;

		/** 빈 위치 검색 */
		if (FindAvailablePosition(
			NewSkillEntry.BaseItem.InstanceId, MainInventory.ContainerId, FoundGridX, FoundGridY) == false)
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

		/** 메인 인벤토리에 아이템 추가 처리 후 메세지 발송*/
		MainInventory.ContainedItemIds.Add(EditableSkill->BaseItem.InstanceId);
		PublishInventoryUpdatedMessage(MainInventory.ContainerId);
		return EditableSkill->BaseItem.InstanceId;
	}

	/** 새로운 아이템 엔트리 생성 */
	FSMItemInstanceData NewItemEntry;
	NewItemEntry.InstanceId = InDropPayload.InstanceId;
	NewItemEntry.ItemType = InDropPayload.ItemType;
	NewItemEntry.Definition = InDropPayload.GetDefinition();
	NewItemEntry.ParentContainerId = MainInventory.ContainerId;
	NewItemEntry.GridX = 0;
	NewItemEntry.GridY = 0;
	NewItemEntry.Rotation = InDropPayload.GetRotation();
	NewItemEntry.bLocked = false;

	ItemEntries.Add(NewItemEntry);

	int32 FoundGridX = 0;
	int32 FoundGridY = 0;

	/** 빈 위치 검색 */
	if (FindAvailablePosition(
		NewItemEntry.InstanceId, MainInventory.ContainerId, FoundGridX, FoundGridY) == false)
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
	/** TODO: 아이템 제거 처리 */
	return false;
}

bool USMInventoryComponent::DropItem(const FGuid& InItemInstanceId)
{
	/** TODO: 드랍 가능 여부 검사 */
	/** TODO: Payload 생성 */
	/** TODO: 월드 드랍 액터 생성 */
	/** TODO: 인벤토리 제거 처리 */
	return false;
}

bool USMInventoryComponent::MoveItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX,
                                     int32 InGridY)
{
	/** TODO: 배치 가능 여부 검사 및 이동 처리 */
	return false;
}

bool USMInventoryComponent::RotateItem(const FGuid& InItemInstanceId)
{
	/** TODO: 현재 회전값 기준 회전 처리 */
	return false;
}

bool USMInventoryComponent::SetItemRotation(const FGuid& InItemInstanceId, ESMGridRotation InRotation)
{
	/** TODO: 회전값 직접 설정 처리 */
	return false;
}

bool USMInventoryComponent::CanPlaceItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX,
                                         int32 InGridY) const
{
	const FSMGridContainerState* TargetContainer = FindContainer(InTargetContainerId);
	if (TargetContainer == nullptr)
	{
		return false;
	}

	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	ESMGridRotation RotationToUse = ESMGridRotation::Rot0;

	if (ItemData != nullptr)
	{
		RotationToUse = ItemData->Rotation;
	}
	else
	{
		const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
		if (SkillData == nullptr)
		{
			return false;
		}

		RotationToUse = SkillData->BaseItem.Rotation;
	}

	TArray<FIntPoint> OccupiedCells;
	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, RotationToUse, OccupiedCells) == false)
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

	if (HasPlacementConflict(InItemInstanceId, InTargetContainerId, InGridX, InGridY))
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

	for (int32 Y = 0; Y < TargetContainer->ValidMask.Height; ++Y)
	{
		for (int32 X = 0; X < TargetContainer->ValidMask.Width; ++X)
		{
			if (CanPlaceItem(InItemInstanceId, InTargetContainerId, X, Y))
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
                                                 int32 InGridX, int32 InGridY) const
{
	TArray<FIntPoint> TargetCells;

	const FSMItemInstanceData* ItemData = FindItem(InItemInstanceId);
	ESMGridRotation RotationToUse = ESMGridRotation::Rot0;

	if (ItemData != nullptr)
	{
		RotationToUse = ItemData->Rotation;
	}
	else
	{
		const FSMSkillItemInstanceData* SkillData = FindSkill(InItemInstanceId);
		if (SkillData == nullptr)
		{
			return true;
		}

		RotationToUse = SkillData->BaseItem.Rotation;
	}

	if (BuildOccupiedCells(InItemInstanceId, InGridX, InGridY, RotationToUse, TargetCells) == false)
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
	/** TODO: EmbeddedItemIds 기반 비어 있음 검사 */
	return false;
}

bool USMInventoryComponent::CanDropItemInternal(const FGuid& InItemInstanceId) const
{
	/** TODO: DropRule 검사 */
	return false;
}

bool USMInventoryComponent::BuildDropPayload(const FGuid& InItemInstanceId, FSMItemDropPayload& OutPayload) const
{
	/** TODO: 드랍 복원 데이터 구성 */
	return false;
}

bool USMInventoryComponent::SpawnDropActorFromPayload(const FSMItemDropPayload& InPayload)
{
	/** TODO: DropActorClass 기반 액터 생성 */
	return false;
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
