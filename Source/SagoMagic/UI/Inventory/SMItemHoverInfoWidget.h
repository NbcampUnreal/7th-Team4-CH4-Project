#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMItemHoverInfoWidget.generated.h"

class USMInventoryComponent;


/**
 * 인벤토리 아이템 호버 정보 위젯 정의 파일
 *
 * 포함 내용:
 * - 표시 대상 아이템 인스턴스 ID
 * - 인벤토리 컴포넌트 참조
 * - 표시 이름
 * - 설명
 * - 강조 색상
 * - 아이템 종류
 * - 표시 위치
 * - 표시 여부
 *
 * 역할:
 * - 현재 호버 중인 아이템의 정보를 별도 패널로 표시
 */

/** 인벤토리 아이템 호버 정보 위젯 */
UCLASS()
class SAGOMAGIC_API USMItemHoverInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMItemHoverInfoWidget(const FObjectInitializer& ObjectInitializer);

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 표시 대상 아이템 인스턴스 ID Getter */
	const FGuid& GetItemInstanceId() const
	{
		return ItemInstanceId;
	}

	/** 표시 이름 Getter */
	const FText& GetDisplayName() const
	{
		return DisplayName;
	}

	/** 설명 Getter */
	const FText& GetDescription() const
	{
		return Description;
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

	/** 표시 위치 Getter */
	const FVector2D& GetScreenPosition() const
	{
		return ScreenPosition;
	}

	/** 표시 여부 Getter */
	bool IsShowingItemInfo() const
	{
		return bIsShowingItemInfo;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 호버 정보 위젯 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Item Hover Info Widget")
	void InitializeHoverInfoWidget(USMInventoryComponent* InInventoryComponent);

	/** 아이템 정보 표시 요청 */
	UFUNCTION(BlueprintCallable, Category="Item Hover Info Widget")
	void ShowItemInfo(const FGuid& InItemInstanceId, FVector2D InScreenPosition);

	/** 표시 위치 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Item Hover Info Widget")
	void UpdateScreenPosition(FVector2D InScreenPosition);

	/** 아이템 정보 숨김 요청 */
	UFUNCTION(BlueprintCallable, Category="Item Hover Info Widget")
	void HideItemInfo();

	/** 현재 아이템 정보 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Item Hover Info Widget")
	void RefreshItemInfo();

protected:
	/** 현재 인벤토리 데이터 기준 표시 정보 동기화 */
	void SyncFromInventoryComponent();

	/** 호버 정보 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Item Hover Info Widget")
	void BP_OnHoverInfoUpdated();

public:
protected:
	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 표시 대상 아이템 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	FGuid ItemInstanceId;

	/** 표시 이름 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	FText Description;

	/** 강조 색상 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	FLinearColor AccentColor;

	/** 아이템 종류 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	ESMItemType DisplayItemType;

	/** 현재 표시 위치 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	FVector2D ScreenPosition;

	/** 현재 표시 여부 */
	UPROPERTY(BlueprintReadOnly, Category="Item Hover Info Widget")
	bool bIsShowingItemInfo;

private:
};
