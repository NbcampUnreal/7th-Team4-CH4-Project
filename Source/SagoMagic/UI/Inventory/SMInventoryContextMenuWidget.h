#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryContextMenuWidget.generated.h"

class USMInventoryComponent;
class USMPlayerInventoryPanelWidget;


/**
 * 인벤토리 컨텍스트 메뉴 위젯 정의 파일
 *
 * 포함 내용:
 * - 대상 아이템 인스턴스 ID
 * - 인벤토리 컴포넌트 참조
 * - 드랍 요청 함수
 * - 해제 요청 함수
 *
 * 역할:
 * - 아이템 우클릭 메뉴 처리
 */

/** 인벤토리 컨텍스트 메뉴 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryContextMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryContextMenuWidget(const FObjectInitializer& ObjectInitializer);

	/** 대상 아이템 인스턴스 ID Getter */
	const FGuid& GetItemInstanceId() const
	{
		return ItemInstanceId;
	}

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 스킬 인벤토리 열기 가능 여부 Getter */
	UFUNCTION(BlueprintPure, Category="Inventory Context Menu Widget")
	bool CanOpenSkillInventory() const;

	/** 드랍 가능 여부 Getter */
	UFUNCTION(BlueprintPure, Category="Inventory Context Menu Widget")
	bool CanDropItem() const;

	/** 즉시 삭제 가능 여부 Getter */
	UFUNCTION(BlueprintPure, Category="Inventory Context Menu Widget")
	bool CanDeleteItem() const;

	/** 대상 아이템이 퀵슬롯 장착 상태인지 Getter */
	UFUNCTION(BlueprintPure, Category="Inventory Context Menu Widget")
	bool IsEquippedInQuickSlot() const;

	/** 대상 아이템 인스턴스 ID Setter */
	void SetItemInstanceId(const FGuid& InItemInstanceId)
	{
		ItemInstanceId = InItemInstanceId;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

	/** 소유 패널 위젯 Setter */
	void SetOwningPanelWidget(USMPlayerInventoryPanelWidget* InOwningPanelWidget)
	{
		OwningPanelWidget = InOwningPanelWidget;
	}

public:
	/** 컨텍스트 메뉴 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void InitializeContextMenu(const FGuid& InItemInstanceId, USMInventoryComponent* InInventoryComponent);

	/** 아이템 드랍 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void RequestDropItem();

	/** 스킬 인벤토리 열기 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void RequestOpenSkillInventory();

	/** 아이템 즉시 삭제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void RequestDeleteItem();

	/** 내부 장착 아이템 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void RequestDetachEmbeddedItem();

	/** 스킬 퀵슬롯 장착/해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Context Menu Widget")
	void RequestToggleSkillQuickSlot();

protected:
	/** 컨텍스트 메뉴 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Context Menu Widget")
	void BP_OnContextMenuUpdated();

private:

public:

protected:
	/** 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	FGuid ItemInstanceId;

	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 소유 패널 위젯 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	TObjectPtr<USMPlayerInventoryPanelWidget> OwningPanelWidget;

	/** 스킬 인벤토리 열기 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	bool bCanOpenSkillInventory;

	/** 드랍 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	bool bCanDropItem;

	/** 즉시 삭제 가능 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	bool bCanDeleteItem;

	/** 대상 아이템의 퀵슬롯 장착 상태 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Context Menu Widget")
	bool bEquippedInQuickSlot;

private:
};
