#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMQuickSlotBarWidget.generated.h"

class USMInventoryComponent;
class UBorder;
class UCanvasPanel;
class UDragDropOperation;
class USMDragItemPreviewWidget;
class USMInventoryDragDropOperation;


/**
 * 퀵슬롯 바 위젯 정의 파일
 *
 * 포함 내용:
 * - 인벤토리 컴포넌트 참조
 * - 현재 활성 슬롯 인덱스
 * - 첫 번째 슬롯 스킬 인스턴스 ID
 * - 두 번째 슬롯 스킬 인스턴스 ID
 *
 * 역할:
 * - 퀵슬롯 2개 상태와 활성 슬롯 표시
 */

/** 퀵슬롯 바 위젯 */
UCLASS()
class SAGOMAGIC_API USMQuickSlotBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMQuickSlotBarWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** NativeDestruct 오버라이드 */
	virtual void NativeDestruct() override;

	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 활성 슬롯 인덱스 Getter */
	int32 GetActiveSlotIndex() const
	{
		return ActiveSlotIndex;
	}

	/** 첫 번째 슬롯 스킬 인스턴스 ID Getter */
	const FGuid& GetSlot1SkillId() const
	{
		return Slot1SkillId;
	}

	/** 두 번째 슬롯 스킬 인스턴스 ID Getter */
	const FGuid& GetSlot2SkillId() const
	{
		return Slot2SkillId;
	}

	/** 퀵슬롯 엔트리 배열 Getter */
	const TArray<FSMQuickSlotEntry>& GetSlots() const
	{
		return Slots;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 퀵슬롯 바 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void InitializeQuickSlotBarWidget(USMInventoryComponent* InInventoryComponent);

	/** 퀵슬롯 바 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void RefreshQuickSlotBar();

	/** 첫 번째 슬롯 활성화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void ActivateFirstSlot();

	/** 두 번째 슬롯 활성화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void ActivateSecondSlot();

protected:
	/** 퀵슬롯 상태 동기화 */
	void SyncFromInventoryComponent();

	/** 퀵슬롯 슬롯 프리뷰 재구성 */
	void RebuildSlotPreviewVisuals();

	/** 화면 좌표 기준 슬롯 인덱스 계산 */
	bool FindSlotIndexAtScreenPosition(const FVector2D& InScreenPosition, int32& OutSlotIndex) const;

	/** 화면 좌표 기준 슬롯 아이템 조회 */
	bool FindSlotItemAtScreenPosition(const FVector2D& InScreenPosition, int32& OutSlotIndex, FGuid& OutSkillInstanceId) const;

	/** 현재 드래그 오퍼레이션의 퀵슬롯 장착 가능 여부 검사 */
	bool CanEquipDraggedSkillToQuickSlot(USMInventoryDragDropOperation* InInventoryOperation, int32 InTargetSlotIndex) const;

	/** 퀵슬롯 스킬 드래그 드롭 오퍼레이션 생성 */
	UDragDropOperation* CreateDragDropOperationForQuickSlotSkill(int32 InSlotIndex, const FGeometry& InGeometry,
	                                                             const FPointerEvent& InMouseEvent);

	/** 퀵슬롯 갱신 메시지 리스너 등록 */
	void RegisterQuickSlotMessageListener();

	/** 퀵슬롯 갱신 메시지 리스너 해제 */
	void UnregisterQuickSlotMessageListener();

	/** 퀵슬롯 갱신 메시지 수신 */
	void HandleQuickSlotUpdatedMessage(FGameplayTag InChannel, const FSMQuickSlotUpdatedMessage& InMessage);

	/** 퀵슬롯 바 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Quick Slot Bar Widget")
	void BP_OnQuickSlotBarUpdated();

public:
protected:
	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 현재 활성 슬롯 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	int32 ActiveSlotIndex;

	/** 첫 번째 슬롯 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	FGuid Slot1SkillId;

	/** 두 번째 슬롯 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	FGuid Slot2SkillId;

	/** 퀵슬롯 엔트리 배열 */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TArray<FSMQuickSlotEntry> Slots;

	/** 첫 번째 슬롯 프리뷰 캔버스 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UCanvasPanel> Slot1_PreviewCanvas;

	/** 두 번째 슬롯 프리뷰 캔버스 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UCanvasPanel> Slot2_PreviewCanvas;

	/** 첫 번째 슬롯 활성 테두리 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UBorder> Slot1_ActiveOutline;

	/** 두 번째 슬롯 활성 테두리 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UBorder> Slot2_ActiveOutline;

	/** 첫 번째 슬롯 기본 배경 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UBorder> Slot1_BaseBackground;

	/** 두 번째 슬롯 기본 배경 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<UBorder> Slot2_BaseBackground;

	/** 슬롯 내부 프리뷰 가용 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quick Slot Bar Widget")
	FVector2D PreviewAreaSize = FVector2D(72.0f, 72.0f);

	/** 프리뷰 셀 최소 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quick Slot Bar Widget")
	float MinPreviewCellSize = 8.0f;

	/** 프리뷰 셀 최대 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quick Slot Bar Widget")
	float MaxPreviewCellSize = 48.0f;

	/** 프리뷰 셀 패딩 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quick Slot Bar Widget")
	float PreviewCellPadding = 2.0f;

	/** 드래그 미리보기 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TSubclassOf<USMDragItemPreviewWidget> DragPreviewWidgetClass;

private:
	FGameplayMessageListenerHandle QuickSlotUpdatedListenerHandle;
	int32 PendingDragSlotIndex = INDEX_NONE;
};
