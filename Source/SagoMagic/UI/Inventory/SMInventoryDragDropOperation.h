#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Blueprint/DragDropOperation.h"
#include "SMInventoryDragDropOperation.generated.h"

class USMDragItemPreviewWidget;
class USMPlayerInventoryPanelWidget;


/**
 * 인벤토리 드래그 드롭 데이터 클래스 정의 파일
 *
 * 포함 내용:
 * - 드래그 대상 아이템 인스턴스 ID
 * - 출발 컨테이너 ID
 * - 출발 Grid 좌표
 * - 드래그 시작 회전값
 * - 현재 드래그 회전값
 * - 드래그 미리보기 위젯 참조
 *
 * 역할:
 * - 인벤토리 드래그 드롭 중 필요한 데이터 보관
 */

/** 인벤토리 드래그 드롭 데이터 클래스 */
UCLASS()
class SAGOMAGIC_API USMInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryDragDropOperation();

	/** 아이템 인스턴스 ID Getter */
	const FGuid& GetItemInstanceId() const
	{
		return ItemInstanceId;
	}

	/** 출발 컨테이너 ID Getter */
	const FGuid& GetSourceContainerId() const
	{
		return SourceContainerId;
	}

	/** 출발 Grid X Getter */
	int32 GetSourceGridX() const
	{
		return SourceGridX;
	}

	/** 출발 Grid Y Getter */
	int32 GetSourceGridY() const
	{
		return SourceGridY;
	}

	/** 드래그 시작 회전값 Getter */
	ESMGridRotation GetStartRotation() const
	{
		return StartRotation;
	}

	/** 현재 드래그 회전값 Getter */
	ESMGridRotation GetCurrentRotation() const
	{
		return CurrentRotation;
	}

	/** 기준 Shape 로컬 X Getter */
	int32 GetPivotShapeLocalX() const
	{
		return PivotShapeLocalX;
	}

	/** 기준 Shape 로컬 Y Getter */
	int32 GetPivotShapeLocalY() const
	{
		return PivotShapeLocalY;
	}

	/** Shape 가로 셀 수 Getter */
	int32 GetShapeWidth() const
	{
		return ShapeWidth;
	}

	/** Shape 세로 셀 수 Getter */
	int32 GetShapeHeight() const
	{
		return ShapeHeight;
	}

	/** 드래그 미리보기 위젯 Getter */
	USMDragItemPreviewWidget* GetDragPreviewWidget() const
	{
		return DragPreviewWidget;
	}

	/** 드래그 프리뷰 소유 패널 Getter */
	USMPlayerInventoryPanelWidget* GetOwningInventoryPanel() const
	{
		return OwningInventoryPanel;
	}

	/** 아이템 인스턴스 ID Setter */
	void SetItemInstanceId(const FGuid& InItemInstanceId)
	{
		ItemInstanceId = InItemInstanceId;
	}

	/** 출발 컨테이너 ID Setter */
	void SetSourceContainerId(const FGuid& InSourceContainerId)
	{
		SourceContainerId = InSourceContainerId;
	}

	/** 출발 Grid X Setter */
	void SetSourceGridX(const int32 InSourceGridX)
	{
		SourceGridX = InSourceGridX;
	}

	/** 출발 Grid Y Setter */
	void SetSourceGridY(const int32 InSourceGridY)
	{
		SourceGridY = InSourceGridY;
	}

	/** 드래그 시작 회전값 Setter */
	void SetStartRotation(const ESMGridRotation InStartRotation)
	{
		StartRotation = InStartRotation;
	}

	/** 현재 드래그 회전값 Setter */
	void SetCurrentRotation(const ESMGridRotation InCurrentRotation)
	{
		CurrentRotation = InCurrentRotation;
	}

	/** 기준 Shape 로컬 X Setter */
	void SetPivotShapeLocalX(int32 InPivotShapeLocalX)
	{
		PivotShapeLocalX = InPivotShapeLocalX;
	}

	/** 기준 Shape 로컬 Y Setter */
	void SetPivotShapeLocalY(int32 InPivotShapeLocalY)
	{
		PivotShapeLocalY = InPivotShapeLocalY;
	}

	/** Shape 가로 셀 수 Setter */
	void SetShapeWidth(int32 InShapeWidth)
	{
		ShapeWidth = InShapeWidth;
	}

	/** Shape 세로 셀 수 Setter */
	void SetShapeHeight(int32 InShapeHeight)
	{
		ShapeHeight = InShapeHeight;
	}

	/** 드래그 미리보기 위젯 Setter */
	void SetDragPreviewWidget(USMDragItemPreviewWidget* InDragPreviewWidget)
	{
		DragPreviewWidget = InDragPreviewWidget;
	}

	/** 드래그 프리뷰 소유 패널 Setter */
	void SetOwningInventoryPanel(USMPlayerInventoryPanelWidget* InOwningInventoryPanel)
	{
		OwningInventoryPanel = InOwningInventoryPanel;
	}

public:
	/** 드래그 데이터 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	void InitializeOperation(
		const FGuid& InItemInstanceId,
		const FGuid& InSourceContainerId,
		int32 InSourceGridX,
		int32 InSourceGridY,
		ESMGridRotation InStartRotation,
		int32 InPivotShapeLocalX,
		int32 InPivotShapeLocalY,
		int32 InShapeWidth,
		int32 InShapeHeight,
		FVector2D InPivotCellFraction,
		USMDragItemPreviewWidget* InDragPreviewWidget);

	/** 현재 드래그 회전값 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	void UpdateCurrentRotation(ESMGridRotation InCurrentRotation);

	virtual void Drop_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;
	virtual void Dragged_Implementation(const FPointerEvent& PointerEvent) override;

	/** 드래그 대상 아이템 ID 유효성 검사 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	bool HasValidItemInstanceId() const;

	/** 현재 회전 기준 드래그 기준 셀 오프셋 계산 */
	bool CalculateCurrentPivotOffset(int32& OutPivotOffsetX, int32& OutPivotOffsetY) const;

private:
	bool GetCurrentDimensions(int32& OutWidth, int32& OutHeight) const;
	bool CalculatePointerLocalFromPivotData(ESMGridRotation InRotation, FVector2D& OutPointerLocal) const;
	void UpdateDragVisualOffset();

public:
	/** 드래그 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	FGuid ItemInstanceId;

	/** 드래그 출발 컨테이너 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	FGuid SourceContainerId;

	/** 드래그 출발 Grid X 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 SourceGridX;

	/** 드래그 출발 Grid Y 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 SourceGridY;

	/** 드래그 시작 시 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	ESMGridRotation StartRotation;

	/** 현재 드래그 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	ESMGridRotation CurrentRotation;

	/** 기준 Shape 로컬 X 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 PivotShapeLocalX;

	/** 기준 Shape 로컬 Y 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 PivotShapeLocalY;

	/** Shape 가로 셀 수 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 ShapeWidth;

	/** Shape 세로 셀 수 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 ShapeHeight;

	/** 기준 셀 내부 클릭 비율 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	FVector2D PivotCellFraction;

	/** 드래그 시작 시점 커서의 아이템 중심 기준 오프셋 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	FVector2D PointerFromCenter;

	/** 드래그 미리보기 위젯 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	TObjectPtr<USMDragItemPreviewWidget> DragPreviewWidget;

	/** 드래그 프리뷰 소유 패널 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	TObjectPtr<USMPlayerInventoryPanelWidget> OwningInventoryPanel;

protected:

private:
};
