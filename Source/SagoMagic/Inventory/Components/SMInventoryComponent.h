#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemDropTypes.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"
#include "SMInventoryComponent.generated.h"

class USMItemDefinition;
class USMIF_GemModifier;
class ASMBaseItemDropActor;

/**
 * 인벤토리 시스템 핵심 로직 컴포넌트 정의 파일
 *
 * 포함 내용:
 * - 메인 인벤토리 상태
 * - 퀵슬롯 상태
 * - 아이템 레지스트리
 * - 스킬 레지스트리
 * - 스킬 내부 컨테이너 상태
 * - 이동, 회전, 장착, 해제, 드랍, 요약 계산 함수
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

	/** 일반 아이템 레지스트리 Getter */
	const TMap<FGuid, FSMItemInstanceData>& GetItemRegistry() const
	{
		return ItemRegistry;
	}

	/** 스킬 아이템 레지스트리 Getter */
	const TMap<FGuid, FSMSkillItemInstanceData>& GetSkillRegistry() const
	{
		return SkillRegistry;
	}

	/** 스킬 내부 컨테이너 레지스트리 Getter */
	const TMap<FGuid, FSMGridContainerState>& GetSkillInternalContainers() const
	{
		return SkillInternalContainers;
	}

public:
	/** 아이템 정의 기반 아이템 추가 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	FGuid AddItemFromDefinition(const TSoftObjectPtr<USMItemDefinition>& InItemDefinition);

	/** 아이템 제거 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(const FGuid& InItemInstanceId);

	/** 아이템 월드 드랍 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool DropItem(const FGuid& InItemInstanceId);

	/** 아이템 이동 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool MoveItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY);

	/** 아이템 회전 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RotateItem(const FGuid& InItemInstanceId);

	/** 아이템 회전값 직접 설정 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool SetItemRotation(const FGuid& InItemInstanceId, int32 InRotation);

	/** 배치 가능 여부 검사 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool CanPlaceItem(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY) const;

	/** 빈 위치 탐색 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool FindAvailablePosition(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32& OutGridX, int32& OutGridY) const;

	/** 젬 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool AttachGemToSkill(const FGuid& InGemInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY);

	/** 동일 이름 스킬 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool AttachSkillToSkill(const FGuid& InMaterialSkillInstanceId, const FGuid& InTargetSkillInstanceId, int32 InGridX, int32 InGridY);

	/** 내부 장착 아이템 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Skill")
	bool DetachEmbeddedItem(const FGuid& InEmbeddedItemInstanceId);

	/** 퀵슬롯 장착 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool EquipSkillToQuickSlot(const FGuid& InSkillInstanceId, int32 InSlotIndex);

	/** 퀵슬롯 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|QuickSlot")
	bool UnequipSkillFromQuickSlot(int32 InSlotIndex);

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

	/** 스킬 요약 데이터 조회 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory|Query")
	bool GetSkillSummary(const FGuid& InSkillInstanceId, FSMCompiledSkillSummary& OutSummary) const;

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

	/** 아이템 정의 로드 */
	const USMItemDefinition* ResolveItemDefinition(const FSMItemInstanceData& InItemData) const;

protected:
	/** 일반 아이템 수정가능 포인터 조회 */
	FSMItemInstanceData* FindEditableItem(const FGuid& InItemInstanceId);

	/** 스킬 아이템 수정가능 포인터 조회 */
	FSMSkillItemInstanceData* FindEditableSkill(const FGuid& InSkillInstanceId);

	/** 컨테이너 수정가능 포인터 조회 */
	FSMGridContainerState* FindEditableContainer(const FGuid& InContainerId);

	/** 메인 인벤토리 초기화 */
	void InitializeMainInventory();

	/** 스킬 내부 컨테이너 생성 */
	bool CreateSkillInternalContainer(const FGuid& InSkillInstanceId, const FSMGridMaskData& InInternalMask, FGuid& OutCreatedContainerId);

	/** 장착 대상 스킬 태그 충족 여부 검사 */
	bool CanApplyGemToSkillByTags(const USMIF_GemModifier* InGemModifierFragment, const USMItemDefinition* InTargetSkillDefinition) const;

private:
	/** 점유 셀 계산 */
	bool BuildOccupiedCells(const FGuid& InItemInstanceId, int32 InGridX, int32 InGridY, int32 InRotation, TArray<FIntPoint>& OutOccupiedCells) const;

	/** 컨테이너 충돌 여부 검사 */
	bool HasPlacementConflict(const FGuid& InItemInstanceId, const FGuid& InTargetContainerId, int32 InGridX, int32 InGridY) const;

	/** 동일 이름 스킬 여부 검사 */
	bool IsSameNamedSkill(const FGuid& InAItemInstanceId, const FGuid& InBItemInstanceId) const;

	/** 빈 스킬 여부 검사 */
	bool IsSkillActuallyEmpty(const FGuid& InSkillInstanceId) const;

	/** 드랍 가능 여부 검사 */
	bool CanDropItemInternal(const FGuid& InItemInstanceId) const;

	/** 드랍 Payload 생성 */
	bool BuildDropPayload(const FGuid& InItemInstanceId, FSMItemDropPayload& OutPayload) const;

	/** 월드 드랍 액터 생성 */
	bool SpawnDropActorFromPayload(const FSMItemDropPayload& InPayload);

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

public:
	/** 기본 메인 인벤토리 마스크 설정값 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory|Config")
	FSMGridMaskData DefaultMainInventoryMask;

	/** 기본 드랍 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory|Config")
	TSubclassOf<ASMBaseItemDropActor> DefaultDropActorClass;

protected:
	/** 메인 인벤토리 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly, Category="Inventory|Runtime")
	FSMGridContainerState MainInventory;

	/** 퀵슬롯 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly, Category="Inventory|Runtime")
	FSMQuickSlotSetState QuickSlots;

	/** 일반 아이템 레지스트리 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly, Category="Inventory|Runtime")
	TMap<FGuid, FSMItemInstanceData> ItemRegistry;

	/** 스킬 아이템 레지스트리 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly, Category="Inventory|Runtime")
	TMap<FGuid, FSMSkillItemInstanceData> SkillRegistry;

	/** 스킬 내부 컨테이너 레지스트리 */
	UPROPERTY(ReplicatedUsing=OnRep_InventoryStateChanged, VisibleAnywhere, BlueprintReadOnly, Category="Inventory|Runtime")
	TMap<FGuid, FSMGridContainerState> SkillInternalContainers;
};
