#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryCellWidget.generated.h"

class UDragDropOperation;


/**
 * 인벤토리 셀 위젯 정의 파일
 *
 * 포함 내용:
 * - 셀 Grid 좌표
 * - 셀 사용 가능 여부
 * - 마우스 호버 여부
 * - 배치 가능 강조 여부
 * - 배치 불가 강조 여부
 * - 현재 셀 점유 아이템 인스턴스 ID
 *
 * 역할:
 * - 인벤토리 격자의 한 칸 표시와 입력 처리 담당
 */

/** 인벤토리 셀 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryCellWidget(const FObjectInitializer& ObjectInitializer);

	/** 마우스 진입 처리 오버라이드 */
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 마우스 이동 처리 오버라이드 */
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 마우스 이탈 처리 오버라이드 */
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	/** 마우스 버튼 입력 처리 오버라이드 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 드래그 시작 처리 오버라이드 */
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;

	/** 드래그 취소 처리 오버라이드 */
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	/** Grid X Getter */
	int32 GetGridX() const
	{
		return GridX;
	}

	/** Grid Y Getter */
	int32 GetGridY() const
	{
		return GridY;
	}

	/** 셀 사용 가능 여부 Getter */
	bool IsCellEnabled() const
	{
		return bCellEnabled;
	}

	/** 마우스 호버 여부 Getter */
	bool IsHoveredCell() const
	{
		return bHoveredCell;
	}

	/** 배치 가능 강조 여부 Getter */
	bool IsPlaceableHighlighted() const
	{
		return bPlaceableHighlighted;
	}

	/** 배치 불가 강조 여부 Getter */
	bool IsBlockedHighlighted() const
	{
		return bBlockedHighlighted;
	}

	/** 점유 아이템 인스턴스 ID Getter */
	const FGuid& GetOwnerItemInstanceId() const
	{
		return OwnerItemInstanceId;
	}

	/** 점유 여부 Getter */
	bool IsOccupiedCell() const
	{
		return bOccupiedCell;
	}

	/** 점유 셀 강조 색상 Getter */
	const FLinearColor& GetOccupiedAccentColor() const
	{
		return OccupiedAccentColor;
	}

	/** Grid X Setter */
	void SetGridX(const int32 InGridX)
	{
		GridX = InGridX;
	}

	/** Grid Y Setter */
	void SetGridY(const int32 InGridY)
	{
		GridY = InGridY;
	}

	/** 셀 사용 가능 여부 Setter */
	void SetCellEnabled(const bool bInCellEnabled)
	{
		bCellEnabled = bInCellEnabled;
	}

	/** 마우스 호버 여부 Setter */
	void SetHoveredCell(const bool bInHoveredCell)
	{
		bHoveredCell = bInHoveredCell;
	}

	/** 배치 가능 강조 여부 Setter */
	void SetPlaceableHighlighted(const bool bInPlaceableHighlighted)
	{
		bPlaceableHighlighted = bInPlaceableHighlighted;
	}

	/** 배치 불가 강조 여부 Setter */
	void SetBlockedHighlighted(const bool bInBlockedHighlighted)
	{
		bBlockedHighlighted = bInBlockedHighlighted;
	}

	/** 점유 아이템 인스턴스 ID Setter */
	void SetOwnerItemInstanceId(const FGuid& InOwnerItemInstanceId)
	{
		OwnerItemInstanceId = InOwnerItemInstanceId;
	}

	/** 점유 여부 Setter */
	void SetOccupiedCell(const bool bInOccupiedCell)
	{
		bOccupiedCell = bInOccupiedCell;
	}

	/** 점유 셀 강조 색상 Setter */
	void SetOccupiedAccentColor(const FLinearColor& InOccupiedAccentColor)
	{
		OccupiedAccentColor = InOccupiedAccentColor;
	}

public:
	/** 셀 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Widget")
	void InitializeCellWidget(int32 InGridX, int32 InGridY, bool bInCellEnabled);

	/** 셀 상태 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Widget")
	void UpdateCellState(
		bool bInCellEnabled,
		bool bInHoveredCell,
		bool bInPlaceableHighlighted,
		bool bInBlockedHighlighted);

	/** 셀 강조 상태 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Widget")
	void ClearHighlightState();

	/** 점유 아이템 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Widget")
	void UpdateOccupiedItem(const FGuid& InOwnerItemInstanceId, const FLinearColor& InOccupiedAccentColor);

protected:
	/** 셀 상태 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Cell Widget")
	void BP_OnCellStateChanged();

public:
protected:
	/** 셀 Grid X 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	int32 GridX;

	/** 셀 Grid Y 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	int32 GridY;

	/** 셀 사용 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	bool bCellEnabled;

	/** 마우스 호버 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	bool bHoveredCell;

	/** 배치 가능 강조 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	bool bPlaceableHighlighted;

	/** 배치 불가 강조 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	bool bBlockedHighlighted;

	/** 현재 셀 점유 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	FGuid OwnerItemInstanceId;

	/** 현재 셀 점유 상태 표시 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	bool bOccupiedCell;

	/** 현재 셀 점유 아이템 강조 색상 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Cell Widget")
	FLinearColor OccupiedAccentColor;

private:
};
