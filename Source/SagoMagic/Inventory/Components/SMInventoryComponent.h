#pragma once

#include "CoreMinimal.h"
#include "Containers/BitArray.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemDropTypes.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"
#include "SMInventoryComponent.generated.h"

class USMItemDefinition;
class USMGemModifierFragment;
class ASMBaseItemDropActor;

/**
 * 인벤토리 시스템 핵심 로직 컴포넌트 정의 파일
 *
 * 포함 내용:
 * - 메인 인벤토리 상태
 * - 퀵슬롯 상태
 * - 일반 아이템 엔트리 배열
 * - 스킬 아이템 엔트리 배열
 * - 스킬 내부 컨테이너 배열
 * - 이동, 장착, 해제, 드랍, 요약 계산 함수
 * - Gameplay Message Subsystem 기반 UI 갱신 함수
 *
 * 역할:
 * - 실제 인벤토리 상태를 소유하고 변경하는 인벤토리 도메인 로직 중심부
 */

/** 인벤토리 핵심 로직 컴포넌트 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryComponent();

	/** BeginPlay 오버라이드 */
	virtual void BeginPlay() override;

	/** 리플리케이션 프로퍼티 등록 오버라이드 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 메인 인벤토리 상태 Getter */
	const FSMGridContainerState& GetMainInventory() const
	{
		return MainInventory;
	}

	/** 퀵슬롯 상태 Getter */
	const FSMQuickSlotSetState& GetQuickSlots() const
	{
		return QuickSlots;
	}

	/** 일반 아이템 엔트리 배열 Getter */
	const TArray<FSMItemInstanceData>& GetItemEntries() const
	{
		return ItemEntries;
	}

	/** 스킬 아이템 엔트리 배열 Getter */
	const TArray<FSMSkillItemInstanceData>& GetSkillEntries() const
	{
		return SkillEntries;
	}

	/** 스킬 내부 컨테이너 배열 Getter */
	const TArray<FSMGridContainerState>& GetSkillInternalContainers() const
	{
		return SkillInternalContainers;
	}

public:
	/** 아이템 정의 기반 아이템 추가 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FGuid AddItemFromDefinition(const TSoftObjectPtr<USMItemDefinition>& InItemDefinition);

	/** 아이템 드랍 페이로드 기반 아이템 추가 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FGuid AddItemFromDropPayload(const FSMItemDropPayload& InDropPayload);

	/** 아이템 제거 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(const FGuid& InItemInstanceId);

	/** 아이템 월드 드랍 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool DropItem(const FGuid& InItemInstanceId, const FTransform& InDropTransform);

	/** 아이템 이동 요청
	 *  드래그 중 결정된 회전값까지 함께 확정합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool MoveItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY,
	              ESMGridRotation InRotation);

	/** 배치 가능 여부 검사 요청
	 *  드래그 중 임시 회전값을 받아 검사합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool CanPlaceItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY,
	                  ESMGridRotation InRotation) const;

	/** 빈 위치 탐색 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool FindAvailablePosition(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32& OutGridX,
	                           int32& OutGridY) const;

	/** 젬 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool AttachGemToSkill(const FGuid& InGemInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX,
	                      int32 InGridY);

	/** 동일 이름 스킬 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool AttachSkillToSkill(const FGuid& InMaterialSkillInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX,
	                        int32 InGridY);

	/** 내부 장착 아이템 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool DetachEmbeddedItem(const FGuid& InEmbeddedItemInstanceId);

	/** 퀵슬롯 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool EquipSkillToQuickSlot(const FGuid& InSkillInstanceId, int32 InSlotIndex);

	/** 빈 퀵슬롯(0 -> 1 순) 자동 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool EquipSkillToFirstAvailableQuickSlot(const FGuid& InSkillInstanceId);

	/** 퀵슬롯 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool UnequipSkillFromQuickSlot(int32 InSlotIndex);

	/** 퀵슬롯 스킬을 메인 인벤토리 지정 위치로 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool UnequipSkillFromQuickSlotToMainInventory(int32 InSlotIndex, int32 InGridX, int32 InGridY,
	                                              ESMGridRotation InRotation);

	/** 활성 퀵슬롯 설정 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	void SetActiveQuickSlot(int32 InSlotIndex);

	/** 스킬 요약 재계산 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool RebuildSkillSummary(const FGuid& InSkillInstanceId);

	/** 일반 아이템 데이터 조회 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	bool GetItemData(const FGuid& InItemInstanceId, FSMItemInstanceData& OutItemData) const;

	/** 스킬 아이템 데이터 조회 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	bool GetSkillData(const FGuid& InSkillInstanceId, FSMSkillItemInstanceData& OutSkillData) const;

	/** 아이템 드랍 가능 여부 조회 요청 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Inventory|Query")
	bool CanDropItem(const FGuid& InItemInstanceId) const;

	/** 현재 활성 퀵슬롯의 스킬 요약 데이터 조회 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	bool GetActiveSkillSummary(FSMCompiledSkillSummary& OutSummary) const;

	/** 현재 활성 퀵슬롯의 스킬 실행 태그 조회 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	FGameplayTag GetActiveSkillTag() const;

	/** 컨테이너 데이터 조회 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	bool GetContainerData(const FGuid& InContainerId, FSMGridContainerState& OutContainerData) const;

private:
	/** 복제 상태 변경 수신 함수 */
	UFUNCTION()
	void OnRep_InventoryStateChanged();

public:
	/** 아이템 존재 여부 검사 */
	bool HasItem(const FGuid& InItemInstanceId) const;

	/** 컨테이너 소유 여부 검사 */
	bool OwnsContainer(const FGuid& InContainerId) const;

	/** 일반 아이템 포인터 조회 */
	const FSMItemInstanceData* FindItem(const FGuid& InItemInstanceId) const;

	/** 스킬 아이템 포인터 조회 */
	const FSMSkillItemInstanceData* FindSkill(const FGuid& InSkillInstanceId) const;

	/** 컨테이너 포인터 조회 */
	const FSMGridContainerState* FindContainer(const FGuid& InContainerId) const;

	/** 퀵슬롯 엔트리 포인터 조회 */
	const FSMQuickSlotEntry* FindQuickSlotEntry(int32 InSlotIndex) const;

	/** 아이템 정의 로드 */
	const USMItemDefinition* ResolveItemDefinition(const FSMItemInstanceData& InItemData) const;

protected:
	/** 일반 아이템 수정가능 포인터 조회 */
	FSMItemInstanceData* FindEditableItem(const FGuid& InItemInstanceId);

	/** 스킬 아이템 수정가능 포인터 조회 */
	FSMSkillItemInstanceData* FindEditableSkill(const FGuid& InSkillInstanceId);

	/** 컨테이너 수정가능 포인터 조회 */
	FSMGridContainerState* FindEditableContainer(const FGuid& InContainerId);

	/** 퀵슬롯 엔트리 수정가능 포인터 조회 */
	FSMQuickSlotEntry* FindEditableQuickSlotEntry(int32 InSlotIndex);

	/** 메인 인벤토리 초기화 */
	void InitializeMainInventory();

	/** 퀵슬롯 상태 초기화 */
	void InitializeQuickSlots();

	/** 스킬 내부 컨테이너 생성 */
	bool CreateSkillInternalContainer(const FGuid& InSkillInstanceId, const FSMGridMaskData& InInternalMask,
	                                  FGuid& OutCreatedContainerId);

	/** 장착 대상 스킬 태그 충족 여부 검사 */
	bool CanApplyGemToSkillByTags(const USMGemModifierFragment* InGemModifierFragment,
	                              const USMItemDefinition* InTargetSkillDefinition) const;

private:
	/** 점유 셀 계산 */
	bool BuildOccupiedCells(const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY, ESMGridRotation InRotation,
	                        TArray<FIntPoint>& OutOccupiedCells) const;

	/** 특정 아이템의 점유 셀을 Bitset에 반영 */
	void AddOccupiedBits(TBitArray<>& InOutOccupiedBits, const FSMGridContainerState& InTargetContainer,
	                     const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY, ESMGridRotation InRotation) const;

	/** 컨테이너 충돌 여부 검사 */
	bool HasPlacementConflict(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX,
	                          int32 InGridY, ESMGridRotation InRotation) const;

	/** 동일 이름 스킬 여부 검사 */
	bool IsSameNamedSkill(const FGuid& InAItemInstanceId, const FGuid& InBItemInstanceId) const;

	/** 빈 스킬 여부 검사 */
	bool IsSkillActuallyEmpty(const FGuid& InSkillInstanceId) const;

	/** 드랍 가능 여부 검사 */
	bool CanDropItemInternal(const FGuid& InItemInstanceId) const;

	/** 드랍 Payload 생성 */
	bool BuildDropPayload(const FGuid& InItemInstanceId, FSMItemDropPayload& OutPayload) const;

	/** 월드 드랍 액터 생성 */
	ASMBaseItemDropActor* SpawnDropActorFromPayload(const FSMItemDropPayload& InPayload,
	                                                const FTransform& InSpawnTransform);

	/** 스킬 요약 계산 */
	bool BuildSkillSummary(const FGuid& InSkillInstanceId, FSMCompiledSkillSummary& OutSummary) const;

	/** 슬롯 인덱스 유효성 검사 */
	bool IsValidQuickSlotIndex(int32 InSlotIndex) const;

	/** 인벤토리 갱신 메시지 발행 */
	void PublishInventoryUpdatedMessage(const FGuid& InContainerId) const;

	/** 스킬 요약 갱신 메시지 발행 */
	void PublishSkillSummaryUpdatedMessage(const FGuid& InSkillInstanceId) const;

	/** 퀵슬롯 갱신 메시지 발행 */
	void PublishQuickSlotUpdatedMessage(int32 InSlotIndex) const;

	/** 루트 스킬 기준 내부 드랍 스냅샷 수집 */
	void CollectNestedDropSnapshots(const FGuid& InParentSkillInstanceId,
	                                TArray<FSMNestedItemDropSnapshot>& OutSnapshots) const;

	/** 내부 드랍 스냅샷 1개를 특정 컨테이너에 복원 */
	FGuid AddNestedSnapshotToContainer(const FSMNestedItemDropSnapshot& InSnapshot,
	                                   const FGuid& InTargetContainerId);

	/** 특정 부모 스킬 아래의 내부 드랍 스냅샷들을 재귀 복원 */
	bool RestoreNestedPayloads(const FSMItemDropPayload& InDropPayload, const FGuid& InParentSkillInstanceId);

	/** 실제 아이템 제거 내부 함수 */
	bool RemoveItemInternal(const FGuid& InItemInstanceId, bool bPublishInventoryMessage);

public:
	/** 기본 메인 인벤토리 마스크 설정값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory|Config")
	FSMGridMaskData DefaultMainInventoryMask;

	/** 기본 드랍 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory|Config")
	TSubclassOf<ASMBaseItemDropActor> DefaultDropActorClass;

protected:
	/** 메인 인벤토리 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly,
		Category="Inventory|Runtime")
	FSMGridContainerState MainInventory;

	/** 퀵슬롯 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly,
		Category="Inventory|Runtime")
	FSMQuickSlotSetState QuickSlots;

	/** 일반 아이템 엔트리 배열 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly,
		Category="Inventory|Runtime")
	TArray<FSMItemInstanceData> ItemEntries;

	/** 스킬 아이템 엔트리 배열 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly,
		Category="Inventory|Runtime")
	TArray<FSMSkillItemInstanceData> SkillEntries;

	/** 스킬 내부 컨테이너 배열 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly,
		Category="Inventory|Runtime")
	TArray<FSMGridContainerState> SkillInternalContainers;
};
