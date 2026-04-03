#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMItemWidget.generated.h"

class UDragDropOperation;
class USMInventoryComponent;
class USMInventoryDragDropOperation;
class USMDragItemPreviewWidget;


/**
 * 인벤토리 아이템 표시 위젯 정의 파일
 *
 * 포함 내용:
 * - 표시 대상 아이템 인스턴스 ID
 * - 소속 컨테이너 ID
 * - 현재 Grid 좌표
 * - 현재 표시 회전값
 * - 드래그 가능 여부
 * - 드래그 시작 처리 함수
 * - 인벤토리 데이터 갱신 함수
 *
 * 역할:
 * - 인벤토리 셀 안에 배치된 개별 아이템 표시와 드래그 시작 처리
 */

/** 인벤토리 아이템 표시 위젯 */
UCLASS()
class SAGOMAGIC_API USMItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMItemWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** 마우스 버튼 입력 처리 오버라이드 */
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 드래그 시작 처리 오버라이드 */
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;

	/** 아이템 인스턴스 ID Getter */
	const FGuid& GetItemInstanceId() const
	{
		return ItemInstanceId;
	}

	/** 소속 컨테이너 ID Getter */
	const FGuid& GetOwningContainerId() const
	{
		return OwningContainerId;
	}

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

	/** 현재 표시 회전값 Getter */
	int32 GetDisplayRotation() const
	{
		return DisplayRotation;
	}

	/** 드래그 가능 여부 Getter */
	bool IsDraggable() const
	{
		return bDraggable;
	}

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 아이템 인스턴스 ID Setter */
	void SetItemInstanceId(const FGuid& InItemInstanceId)
	{
		ItemInstanceId = InItemInstanceId;
	}

	/** 소속 컨테이너 ID Setter */
	void SetOwningContainerId(const FGuid& InOwningContainerId)
	{
		OwningContainerId = InOwningContainerId;
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

	/** 현재 표시 회전값 Setter */
	void SetDisplayRotation(const int32 InDisplayRotation)
	{
		DisplayRotation = InDisplayRotation;
	}

	/** 드래그 가능 여부 Setter */
	void SetDraggable(const bool bInDraggable)
	{
		bDraggable = bInDraggable;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 아이템 위젯 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Item Widget")
	void InitializeItemWidget(
		const FGuid& InItemInstanceId,
		const FGuid& InOwningContainerId,
		int32 InGridX,
		int32 InGridY,
		int32 InDisplayRotation,
		USMInventoryComponent* InInventoryComponent);

	/** 인벤토리 데이터 기준 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Item Widget")
	void RefreshItemWidget();

	/** 드래그 시작 가능 여부 검사 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Item Widget")
	bool CanStartDrag() const;

protected:
	/** 드래그 드롭 오퍼레이션 생성 */
	USMInventoryDragDropOperation* CreateDragDropOperation();

	/** 드래그 미리보기 위젯 생성 */
	USMDragItemPreviewWidget* CreateDragPreviewWidget();

	/** 현재 인벤토리 데이터 반영 */
	void UpdateDisplayFromInventory();

	/** 아이템 위젯 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Item Widget")
	void BP_OnItemWidgetUpdated();

public:
protected:
	/** 표시 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	FGuid ItemInstanceId;

	/** 소속 컨테이너 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	FGuid OwningContainerId;

	/** 현재 Grid X 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	int32 GridX;

	/** 현재 Grid Y 좌표 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	int32 GridY;

	/** 현재 표시 회전값 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	int32 DisplayRotation;

	/** 드래그 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inventory Item Widget")
	bool bDraggable;

	/** 드래그 미리보기 위젯 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Item Widget")
	TSubclassOf<USMDragItemPreviewWidget> DragPreviewWidgetClass;

	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Item Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

private:
};
