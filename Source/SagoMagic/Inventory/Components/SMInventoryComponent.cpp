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

    /** TODO: 기본 인벤토리 초기화 처리 */
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

bool USMInventoryComponent::MoveItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY)
{
    /** TODO: 배치 가능 여부 검사 및 이동 처리 */
    return false;
}

bool USMInventoryComponent::RotateItem(const FGuid& InItemInstanceId)
{
    /** TODO: 현재 회전값 기준 회전 처리 */
    return false;
}

bool USMInventoryComponent::SetItemRotation(const FGuid& InItemInstanceId, int32 InRotation)
{
    /** TODO: 회전값 직접 설정 처리 */
    return false;
}

bool USMInventoryComponent::CanPlaceItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY) const
{
    /** TODO: 마스크/충돌/컨테이너 유효성 검사 */
    return false;
}

bool USMInventoryComponent::FindAvailablePosition(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32& OutGridX, int32& OutGridY) const
{
    /** TODO: 순차 탐색 기반 빈 위치 검색 */
    return false;
}

bool USMInventoryComponent::AttachGemToSkill(const FGuid& InGemInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY)
{
    /** TODO: 장착 가능 여부 검사 */
    /** TODO: 장착 처리 */
    /** TODO: 스킬 요약 재계산 */
    return false;
}

bool USMInventoryComponent::AttachSkillToSkill(const FGuid& InMaterialSkillInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY)
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
    /** TODO: 레지스트리 조회 처리 */
    return false;
}

bool USMInventoryComponent::GetSkillData(const FGuid& InSkillInstanceId, FSMSkillItemInstanceData& OutSkillData) const
{
    /** TODO: 레지스트리 조회 처리 */
    return false;
}

bool USMInventoryComponent::GetSkillSummary(const FGuid& InSkillInstanceId, FSMCompiledSkillSummary& OutSummary) const
{
    /** TODO: 요약 캐시 조회 처리 */
    return false;
}

bool USMInventoryComponent::GetContainerData(const FGuid& InContainerId, FSMGridContainerState& OutContainerData) const
{
    /** TODO: 컨테이너 조회 처리 */
    return false;
}

void USMInventoryComponent::OnRep_InventoryStateChanged()
{
    /** TODO: UI 갱신용 메시지 발행 */
}

bool USMInventoryComponent::HasItem(const FGuid& InItemInstanceId) const
{
    /** TODO: 일반/스킬 레지스트리 포함 여부 검사 */
    return false;
}

bool USMInventoryComponent::OwnsContainer(const FGuid& InContainerId) const
{
    /** TODO: 메인/스킬 내부 컨테이너 소유 여부 검사 */
    return false;
}

const FSMItemInstanceData* USMInventoryComponent::FindItem(const FGuid& InItemInstanceId) const
{
    /** TODO: ItemRegistry 조회 */
    return nullptr;
}

const FSMSkillItemInstanceData* USMInventoryComponent::FindSkill(const FGuid& InSkillInstanceId) const
{
    /** TODO: SkillRegistry 조회 */
    return nullptr;
}

const FSMGridContainerState* USMInventoryComponent::FindContainer(const FGuid& InContainerId) const
{
    /** TODO: MainInventory / SkillInternalContainers 조회 */
    return nullptr;
}

const USMItemDefinition* USMInventoryComponent::ResolveItemDefinition(const FSMItemInstanceData& InItemData) const
{
    /** TODO: Definition 로드 처리 */
    return nullptr;
}

FSMItemInstanceData* USMInventoryComponent::FindEditableItem(const FGuid& InItemInstanceId)
{
    /** TODO: ItemRegistry 조회 */
    return nullptr;
}

FSMSkillItemInstanceData* USMInventoryComponent::FindEditableSkill(const FGuid& InSkillInstanceId)
{
    /** TODO: SkillRegistry 조회 */
    return nullptr;
}

FSMGridContainerState* USMInventoryComponent::FindEditableContainer(const FGuid& InContainerId)
{
    /** TODO: MainInventory / SkillInternalContainers 조회 */
    return nullptr;
}

void USMInventoryComponent::InitializeMainInventory()
{
    /** TODO: DefaultMainInventoryMask 기반 초기화 */
}

bool USMInventoryComponent::CreateSkillInternalContainer(const FGuid& InSkillInstanceId, const FSMGridMaskData& InInternalMask, FGuid& OutCreatedContainerId)
{
    /** TODO: 스킬 내부 컨테이너 생성 처리 */
    return false;
}

bool USMInventoryComponent::CanApplyGemToSkillByTags(const USMGemModifierFragment* InGemModifierFragment, const USMItemDefinition* InTargetSkillDefinition) const
{
    /** TODO: RequiredTargetTags / BlockedTargetTags 검사 */
    return false;
}

bool USMInventoryComponent::BuildOccupiedCells(const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY, int32 InRotation, TArray<FIntPoint>& OutOccupiedCells) const
{
    /** TODO: 비트마스크 기반 점유 셀 계산 */
    return false;
}

bool USMInventoryComponent::HasPlacementConflict(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY) const
{
    /** TODO: 기존 배치 아이템과 충돌 검사 */
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
