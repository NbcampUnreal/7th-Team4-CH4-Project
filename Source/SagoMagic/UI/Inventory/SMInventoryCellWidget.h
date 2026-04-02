#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryCellWidget.generated.h"


/**
 * 인벤토리 셀 위젯 정의 파일
 *
 * 포함 내용:
 * - 셀 Grid 좌표
 * - 셀 사용 가능 여부
 * - 마우스 호버 여부
 * - 배치 가능 강조 여부
 * - 배치 불가 강조 여부
 *
 * 역할:
 * - 인벤토리 격자의 한 칸 표시와 상태 표현 담당
 */

/** 인벤토리 셀 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryCellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryCellWidget(const FObjectInitializer& ObjectInitializer);

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

private:
};
