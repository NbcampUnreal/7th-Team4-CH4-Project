#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "SMInventoryDragDropOperation.generated.h"

class USMDragItemPreviewWidget;


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
	int32 GetStartRotation() const
	{
		return StartRotation;
	}

	/** 현재 드래그 회전값 Getter */
	int32 GetCurrentRotation() const
	{
		return CurrentRotation;
	}

	/** 드래그 미리보기 위젯 Getter */
	USMDragItemPreviewWidget* GetDragPreviewWidget() const
	{
		return DragPreviewWidget;
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
	void SetStartRotation(const int32 InStartRotation)
	{
		StartRotation = InStartRotation;
	}

	/** 현재 드래그 회전값 Setter */
	void SetCurrentRotation(const int32 InCurrentRotation)
	{
		CurrentRotation = InCurrentRotation;
	}

	/** 드래그 미리보기 위젯 Setter */
	void SetDragPreviewWidget(USMDragItemPreviewWidget* InDragPreviewWidget)
	{
		DragPreviewWidget = InDragPreviewWidget;
	}

public:
	/** 드래그 데이터 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	void InitializeOperation(
		const FGuid& InItemInstanceId,
		const FGuid& InSourceContainerId,
		int32 InSourceGridX,
		int32 InSourceGridY,
		int32 InStartRotation,
		USMDragItemPreviewWidget* InDragPreviewWidget);

	/** 현재 드래그 회전값 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	void UpdateCurrentRotation(int32 InCurrentRotation);

	/** 드래그 대상 아이템 ID 유효성 검사 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Drag Drop")
	bool HasValidItemInstanceId() const;

private:

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
	int32 StartRotation;

	/** 현재 드래그 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	int32 CurrentRotation;

	/** 드래그 미리보기 위젯 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Drag Drop")
	TObjectPtr<USMDragItemPreviewWidget> DragPreviewWidget;

protected:

private:
};
