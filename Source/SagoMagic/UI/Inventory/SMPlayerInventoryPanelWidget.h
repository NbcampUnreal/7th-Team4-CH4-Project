#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPlayerInventoryPanelWidget.generated.h"

class USMInventoryComponent;
class USMInventoryGridWidget;
class USMSkillInventoryWidget;
class USMQuickSlotBarWidget;
class USMInventoryContextMenuWidget;


/**
 * 플레이어 인벤토리 패널 위젯 정의 파일
 *
 * 포함 내용:
 * - 인벤토리 컴포넌트 참조
 * - 메인 인벤토리 위젯 참조
 * - 스킬 인벤토리 위젯 참조
 * - 퀵슬롯 바 위젯 참조
 * - 컨텍스트 메뉴 위젯 참조
 * - 현재 선택된 스킬 인스턴스 ID
 *
 * 역할:
 * - 로컬 플레이어 인벤토리 화면 조립과 선택된 스킬 상태 관리
 */

/** 플레이어 인벤토리 패널 위젯 */
UCLASS()
class SAGOMAGIC_API USMPlayerInventoryPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMPlayerInventoryPanelWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 현재 선택된 스킬 인스턴스 ID Getter */
	const FGuid& GetSelectedSkillInstanceId() const
	{
		return SelectedSkillInstanceId;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

	/** 현재 선택된 스킬 인스턴스 ID Setter */
	void SetSelectedSkillInstanceId(const FGuid& InSelectedSkillInstanceId)
	{
		SelectedSkillInstanceId = InSelectedSkillInstanceId;
	}

public:
	/** 패널 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void InitializePanelWidget(USMInventoryComponent* InInventoryComponent);

	/** 패널 전체 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void RefreshPanel();

	/** 스킬 선택 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void SelectSkill(const FGuid& InSkillInstanceId);

	/** 선택된 스킬 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void ClearSelectedSkill();

	/** 스킬 인벤토리 열기 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void OpenSkillInventory(const FGuid& InSkillInstanceId);

	/** 스킬 인벤토리 닫기 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void CloseSkillInventory();

	/** 메인 인벤토리 위젯 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void RefreshMainInventoryWidget();

	/** 스킬 인벤토리 위젯 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void RefreshSkillInventoryWidget();

	/** 퀵슬롯 바 위젯 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	void RefreshQuickSlotBarWidget();

	/** 현재 드래그 중인 아이템 회전 요청 전달 */
	UFUNCTION(BlueprintCallable, Category="Player Inventory Panel Widget")
	bool RequestRotateCurrentDraggedItem();

protected:
	/** 내부 위젯 초기화 */
	void InitializeChildWidgets();

	/** 선택된 스킬 상태 반영 */
	void ApplySelectedSkillState();

	/** 패널 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Player Inventory Panel Widget")
	void BP_OnPanelRefreshed();

public:
protected:
	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Player Inventory Panel Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 메인 인벤토리 위젯 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Player Inventory Panel Widget")
	TObjectPtr<USMInventoryGridWidget> MainInventoryGridWidget;

	/** 스킬 인벤토리 위젯 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Player Inventory Panel Widget")
	TObjectPtr<USMSkillInventoryWidget> SkillInventoryWidget;

	/** 퀵슬롯 바 위젯 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Player Inventory Panel Widget")
	TObjectPtr<USMQuickSlotBarWidget> QuickSlotBarWidget;

	/** 컨텍스트 메뉴 위젯 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Player Inventory Panel Widget")
	TObjectPtr<USMInventoryContextMenuWidget> ContextMenuWidget;

	/** 현재 선택된 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Player Inventory Panel Widget")
	FGuid SelectedSkillInstanceId;

private:
};
