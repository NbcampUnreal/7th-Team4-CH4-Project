#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMDragItemPreviewWidget.generated.h"

class USMInventoryComponent;
class UUniformGridPanel;


/**
 * 인벤토리 드래그 미리보기 위젯 정의 파일
 *
 * 포함 내용:
 * - 미리보기 대상 아이템 인스턴스 ID
 * - 인벤토리 컴포넌트 참조
 * - 표시 이름
 * - 강조 색상
 * - 아이템 종류
 * - 아이템 Shape 마스크
 * - 현재 미리보기 회전값
 * - 현재 배치 가능 여부
 * - 미리보기 갱신 요청 함수
 *
 * 역할:
 * - 드래그 중 마우스를 따라다니는 아이템 미리보기 표시
 */

/** 인벤토리 드래그 미리보기 위젯 */
UCLASS()
class SAGOMAGIC_API USMDragItemPreviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMDragItemPreviewWidget(const FObjectInitializer& ObjectInitializer);

	/** 아이템 인스턴스 ID Getter */
	const FGuid& GetItemInstanceId() const
	{
		return ItemInstanceId;
	}

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 표시 이름 Getter */
	const FText& GetDisplayName() const
	{
		return DisplayName;
	}

	/** 강조 색상 Getter */
	const FLinearColor& GetAccentColor() const
	{
		return AccentColor;
	}

	/** 아이템 종류 Getter */
	ESMItemType GetDisplayItemType() const
	{
		return DisplayItemType;
	}

	/** 아이템 Shape 마스크 Getter */
	const FSMGridMaskData& GetShapeMask() const
	{
		return ShapeMask;
	}

	/** 현재 회전값 Getter */
	ESMGridRotation GetPreviewRotation() const
	{
		return PreviewRotation;
	}

	/** 현재 배치 가능 여부 Getter */
	bool CanPlaceOnCurrentCell() const
	{
		return bCanPlaceOnCurrentCell;
	}

	/** 아이템 인스턴스 ID Setter */
	void SetItemInstanceId(const FGuid& InItemInstanceId)
	{
		ItemInstanceId = InItemInstanceId;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

	/** 현재 회전값 Setter */
	void SetPreviewRotation(const ESMGridRotation InPreviewRotation)
	{
		PreviewRotation = InPreviewRotation;
	}

	/** 현재 배치 가능 여부 Setter */
	void SetCanPlaceOnCurrentCell(const bool bInCanPlaceOnCurrentCell)
	{
		bCanPlaceOnCurrentCell = bInCanPlaceOnCurrentCell;
	}

public:
	/** 미리보기 데이터 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void InitializePreview(const FGuid& InItemInstanceId, ESMGridRotation InPreviewRotation);

	/** 인벤토리 데이터 포함 미리보기 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void InitializePreviewFromInventory(
		const FGuid& InItemInstanceId,
		ESMGridRotation InPreviewRotation,
		USMInventoryComponent* InInventoryComponent);

	/** 미리보기 회전값 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void UpdatePreviewRotation(ESMGridRotation InPreviewRotation);

	/** 현재 배치 가능 여부 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void UpdatePlaceableState(bool bInCanPlaceOnCurrentCell);

protected:
	/** 현재 인벤토리 데이터 기준 미리보기 정보 동기화 */
	void SyncFromInventoryComponent();

	/** 현재 Shape/Rotation 기준 프리뷰 셀 재구성 */
	void RebuildPreviewGrid();

	/** 회전 반영 후 프리뷰 셀 좌표 계산 */
	bool CalculatePreviewCellPosition(int32 InLocalX, int32 InLocalY, int32& OutColumn, int32& OutRow) const;

	/** 미리보기 상태 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Drag Preview")
	void BP_OnPreviewDataChanged();

private:

public:

protected:
	/** 미리보기 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	FGuid ItemInstanceId;

	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 표시 이름 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	FText DisplayName;

	/** 강조 색상 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	FLinearColor AccentColor;

	/** 아이템 종류 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	ESMItemType DisplayItemType;

	/** 아이템 Shape 마스크 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	FSMGridMaskData ShapeMask;

	/** 현재 미리보기 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	ESMGridRotation PreviewRotation;

	/** 현재 배치 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	bool bCanPlaceOnCurrentCell;

	/** 프리뷰 셀 그리드 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Drag Preview")
	TObjectPtr<UUniformGridPanel> PreviewGrid;

	/** 프리뷰 셀 크기 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Drag Preview")
	float PreviewCellSize;

	/** 프리뷰 셀 간격 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Drag Preview")
	float PreviewCellPadding;

private:
};
