#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMQuickSlotBarWidget.generated.h"

class USMInventoryComponent;


/**
 * 퀵슬롯 바 위젯 정의 파일
 *
 * 포함 내용:
 * - 인벤토리 컴포넌트 참조
 * - 현재 활성 슬롯 인덱스
 * - 첫 번째 슬롯 스킬 인스턴스 ID
 * - 두 번째 슬롯 스킬 인스턴스 ID
 *
 * 역할:
 * - 퀵슬롯 2개 상태와 활성 슬롯 표시
 */

/** 퀵슬롯 바 위젯 */
UCLASS()
class SAGOMAGIC_API USMQuickSlotBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMQuickSlotBarWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct 오버라이드 */
	virtual void NativeConstruct() override;

	/** 인벤토리 컴포넌트 Getter */
	USMInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	/** 활성 슬롯 인덱스 Getter */
	int32 GetActiveSlotIndex() const
	{
		return ActiveSlotIndex;
	}

	/** 첫 번째 슬롯 스킬 인스턴스 ID Getter */
	const FGuid& GetSlot1SkillId() const
	{
		return Slot1SkillId;
	}

	/** 두 번째 슬롯 스킬 인스턴스 ID Getter */
	const FGuid& GetSlot2SkillId() const
	{
		return Slot2SkillId;
	}

	/** 인벤토리 컴포넌트 Setter */
	void SetInventoryComponent(USMInventoryComponent* InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
	}

public:
	/** 퀵슬롯 바 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void InitializeQuickSlotBarWidget(USMInventoryComponent* InInventoryComponent);

	/** 퀵슬롯 바 새로고침 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void RefreshQuickSlotBar();

	/** 첫 번째 슬롯 활성화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void ActivateFirstSlot();

	/** 두 번째 슬롯 활성화 요청 */
	UFUNCTION(BlueprintCallable, Category="Quick Slot Bar Widget")
	void ActivateSecondSlot();

protected:
	/** 퀵슬롯 상태 동기화 */
	void SyncFromInventoryComponent();

	/** 퀵슬롯 바 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Quick Slot Bar Widget")
	void BP_OnQuickSlotBarUpdated();

public:
protected:
	/** 인벤토리 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	TObjectPtr<USMInventoryComponent> InventoryComponent;

	/** 현재 활성 슬롯 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	int32 ActiveSlotIndex;

	/** 첫 번째 슬롯 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	FGuid Slot1SkillId;

	/** 두 번째 슬롯 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Quick Slot Bar Widget")
	FGuid Slot2SkillId;

private:
};
