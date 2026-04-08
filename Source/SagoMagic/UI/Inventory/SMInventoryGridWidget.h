#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryGridWidget.generated.h"

class UCanvasPanel;
class UCanvasPanelSlot;
class UDragDropOperation;
class UPanelWidget;
class USMInventoryComponent;
class USMInventoryCellWidget;
class USMItemWidget;
class USMInventoryDragDropOperation;


/**
 * 인벤토리 그리드 위젯 정의 파일
 *
 * 포함 내용:
 * - 표시 대상 컨테이너 ID
 * - 인벤토리 컴포넌트 참조
 * - 셀 위젯 클래스
 * - 아이템 위젯 클래스
 * - 셀 레이어 패널
 * - 아이템 레이어 패널
 * - 셀 위젯 배열
 * - 현재 호버 셀 좌표
 * - 현재 드래그 오퍼레이션 참조
 * - 셀 생성 및 아이템 생성 함수
 * - 셀 및 아이템 배치 함수
 *
 * 역할:
 * - 특정 컨테이너를 격자 형태로 표시하고 드래그 드롭 배치 처리
 */

/** 인벤토리 그리드 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryGridWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryGridWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** 드래그 진입 처리 오버라이드 */
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                              UDragDropOperation* InOperation) override;

	/** 드래그 이탈 처리 오버라이드 */
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** 드롭 처리 오버라이드 */
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	                          UDragDropOperation* InOperation) override;

	/** 컨테이너 ID Getter */
	const FGuid& GetContainerId() const
	{
		return ContainerId;
	}

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 호버 셀 X Getter */
	int32 GetHoveredGridX() const
	{
		return HoveredGridX;
	}

	/** 호버 셀 Y Getter */
	int32 GetHoveredGridY() const
	{
		return HoveredGridY;
	}

	/** 현재 드래그 오퍼레이션 Getter */
	USMInventoryDragDropOperation* GetActiveDragDropOperation() const
	{
		return ActiveDragDropOperation;
	}

	/** 컨테이너 ID Setter */
	void SetContainerId(const FGuid& InContainerId)
	{
		ContainerId = InContainerId;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 그리드 위젯 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Grid Widget")
	void InitializeGridWidget(const FGuid& InContainerId, USMInventoryComponent* InInventoryComponent);

	/** 그리드 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Grid Widget")
	void RefreshGrid();

	/** 현재 드래그 회전 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Grid Widget")
	void RequestRotateDraggedItem();

	/** 현재 호버 셀 상태 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Grid Widget")
	void ClearHoveredCellState();

	/** 현재 드래그 상태 전체 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Grid Widget")
	void ClearActiveDragState();

	/** 특정 아이템 기준 드래그 드롭 오퍼레이션 생성 */
	USMInventoryDragDropOperation* CreateDragDropOperationForItem(
		const FGuid& InItemInstanceId,
		int32 InPivotGridX = INDEX_NONE,
		int32 InPivotGridY = INDEX_NONE,
		FVector2D InPivotCellFraction = FVector2D(0.5f, 0.5f));

	/** 현재 활성 드래그 오퍼레이션 설정 */
	void SetActiveDragOperation(USMInventoryDragDropOperation* InActiveDragDropOperation);

protected:
	/** 드래그 중 목표 Grid 좌표 계산 */
	bool CalculateDropGridPosition(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		int32& OutGridX,
		int32& OutGridY) const;

	/** 드래그 드롭 오퍼레이션 변환 */
	USMInventoryDragDropOperation* GetInventoryDragDropOperation(UDragDropOperation* InOperation) const;

	/** 셀 위젯 생성 */
	void RebuildCellWidgets();

	/** 아이템 위젯 생성 */
	void RebuildItemWidgets();

	/** 셀 위젯 제거 */
	void ClearCellWidgets();

	/** 아이템 위젯 제거 */
	void ClearItemWidgets();

	/** 현재 셀 상태 갱신 */
	void UpdateCellStates();

	/** 드래그 미리보기 상태 갱신 */
	void UpdateDraggedPreviewState(UDragDropOperation* InOperation, bool bInCanPlace);

	/** 셀 위젯 조회 */
	USMInventoryCellWidget* FindCellWidget(int32 InGridX, int32 InGridY) const;

	/** 현재 호버 셀 설정 */
	void SetHoveredCell(int32 InGridX, int32 InGridY);

	/** 아이템 인스턴스 기준 공통 베이스 데이터 조회 */
	bool GetBaseItemData(const FGuid& InItemInstanceId, FSMItemInstanceData& OutBaseItemData) const;

	/** 좌표와 회전값을 명시적으로 받아 점유 셀 계산 */
	bool BuildOccupiedCells(const FSMItemInstanceData& InBaseItemData,
	                        int32 InGridX,
	                        int32 InGridY,
	                        ESMGridRotation InRotation,
	                        TArray<FIntPoint>& OutOccupiedCells) const;

	/** 베이스 아이템 데이터 기준 점유 셀 계산 */
	bool BuildOccupiedCellsFromItemData(const FSMItemInstanceData& InBaseItemData, TArray<FIntPoint>& OutOccupiedCells) const;

	/** 점유 셀 배열 기준 외곽 경계 계산 */
	bool CalculateOccupiedCellBounds(const TArray<FIntPoint>& InOccupiedCells,
	                                 int32& OutMinGridX,
	                                 int32& OutMinGridY,
	                                 int32& OutMaxGridX,
	                                 int32& OutMaxGridY) const;

	/** 베이스 아이템 데이터 기준 셀 표시 색상 조회 */
	bool GetItemAccentColor(const FSMItemInstanceData& InBaseItemData, FLinearColor& OutAccentColor) const;

	/** 베이스 아이템 데이터 기준 셀 점유 정보 반영 */
	void ApplyItemOwnershipToCells(const FSMItemInstanceData& InBaseItemData);

	/** 컨테이너 크기 갱신 */
	void RefreshContainerSize();

	/** 셀 위젯 위치 적용 */
	void ApplyCellWidgetLayout(USMInventoryCellWidget* InCellWidget, int32 InGridX, int32 InGridY);

	/** 아이템 위젯 위치 적용 */
	void ApplyItemWidgetLayout(USMItemWidget* InItemWidget, int32 InGridX, int32 InGridY);

	/** 그리드 좌표를 셀 배열 인덱스로 변환 */
	bool TryGetCellArrayIndex(int32 InGridX, int32 InGridY, int32& OutCellArrayIndex) const;

	/** 그리드 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Grid Widget")
	void BP_OnGridRefreshed();

public:
protected:
	/** 표시 대상 컨테이너 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	FGuid ContainerId;

	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 셀 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Grid Widget")
	TSubclassOf<USMInventoryCellWidget> CellWidgetClass;

	/** 아이템 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Grid Widget")
	TSubclassOf<USMItemWidget> ItemWidgetClass;

	/** 셀 레이어 패널 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Grid Widget")
	TObjectPtr<UPanelWidget> CellLayerPanel;

	/** 아이템 레이어 패널 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Grid Widget")
	TObjectPtr<UPanelWidget> ItemLayerPanel;

	/** 셀 위젯 배열 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	TArray<TObjectPtr<USMInventoryCellWidget>> CellWidgets;

	/** 현재 컨테이너 가로 크기 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	int32 GridWidth;

	/** 현재 컨테이너 세로 크기 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	int32 GridHeight;

	/** 현재 호버 셀 X 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	int32 HoveredGridX;

	/** 현재 호버 셀 Y 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	int32 HoveredGridY;

	/** 현재 드래그 오퍼레이션 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Grid Widget")
	TObjectPtr<USMInventoryDragDropOperation> ActiveDragDropOperation;

private:
};
