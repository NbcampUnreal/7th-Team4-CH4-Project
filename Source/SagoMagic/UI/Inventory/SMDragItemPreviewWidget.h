#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMDragItemPreviewWidget.generated.h"


/**
 * 인벤토리 드래그 미리보기 위젯 정의 파일
 *
 * 포함 내용:
 * - 미리보기 대상 아이템 인스턴스 ID
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

	/** 미리보기 회전값 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void UpdatePreviewRotation(ESMGridRotation InPreviewRotation);

	/** 현재 배치 가능 여부 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Preview")
	void UpdatePlaceableState(bool bInCanPlaceOnCurrentCell);

protected:
	/** 미리보기 상태 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Drag Preview")
	void BP_OnPreviewDataChanged();

private:

public:

protected:
	/** 미리보기 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	FGuid ItemInstanceId;

	/** 현재 미리보기 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	ESMGridRotation PreviewRotation;

	/** 현재 배치 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Preview")
	bool bCanPlaceOnCurrentCell;

private:
};
