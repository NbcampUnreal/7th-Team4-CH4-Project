#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryRootWidget.generated.h"

class UPanelWidget;
class USMInventoryComponent;
class USMPlayerInventoryPanelWidget;


/**
 * 인벤토리 루트 위젯 정의 파일
 *
 * 포함 내용:
 * - 로컬 플레이어 인벤토리 패널 클래스
 * - 패널 부모 레이어 참조
 * - 현재 인벤토리 패널 참조
 * - 인벤토리 컴포넌트 참조
 *
 * 역할:
 * - 로컬 플레이어 인벤토리 화면 전체 루트와 단일 패널 관리
 */

/** 인벤토리 루트 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryRootWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryRootWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 현재 인벤토리 패널 Getter */
	USMPlayerInventoryPanelWidget* GetCurrentPanelWidget() const
	{
		return CurrentPanelWidget;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 루트 위젯 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Root Widget")
	void InitializeRootWidget(USMInventoryComponent* InInventoryComponent);

	/** 현재 인벤토리 패널 생성 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Root Widget")
	USMPlayerInventoryPanelWidget* CreateCurrentPanelWidget();

	/** 현재 인벤토리 패널 제거 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Root Widget")
	void ClearCurrentPanelWidget();

	/** 루트 위젯 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Root Widget")
	void RefreshRootWidget();

protected:
	/** 루트 위젯 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Inventory Root Widget")
	void BP_OnRootWidgetRefreshed();

public:
protected:
	/** 플레이어 인벤토리 패널 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Root Widget")
	TSubclassOf<USMPlayerInventoryPanelWidget> PlayerInventoryPanelWidgetClass;

	/** 패널 부모 레이어 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Root Widget")
	TObjectPtr<UPanelWidget> PanelLayer;

	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Root Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 현재 인벤토리 패널 위젯 */
	UPROPERTY(BlueprintReadOnly, Category="Inventory Root Widget")
	TObjectPtr<USMPlayerInventoryPanelWidget> CurrentPanelWidget;

private:
};
