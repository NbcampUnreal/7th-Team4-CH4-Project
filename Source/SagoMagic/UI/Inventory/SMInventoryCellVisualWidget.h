#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMInventoryCellVisualWidget.generated.h"

class UBorder;


/**
 * 인벤토리 셀 비주얼 위젯 정의 파일
 *
 * 포함 내용:
 * - 외곽선 표시 여부
 * - 점유 Fill 표시 여부
 * - 점유 Fill 강조 색상
 *
 * 역할:
 * - 입력이 없는 공용 셀 비주얼 표시 담당
 */

/** 인벤토리 셀 비주얼 위젯 */
UCLASS()
class SAGOMAGIC_API USMInventoryCellVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInventoryCellVisualWidget(const FObjectInitializer& ObjectInitializer);

	/** NativePreConstruct 오버라이드 */
	virtual void NativePreConstruct() override;

	/** 외곽선 표시 여부 Getter */
	bool IsOutlineVisible() const
	{
		return bShowOutline;
	}

	/** 점유 Fill 표시 여부 Getter */
	bool IsOccupiedFillVisible() const
	{
		return bShowOccupiedFill;
	}

	/** 점유 Fill 강조 색상 Getter */
	const FLinearColor& GetOccupiedAccentColor() const
	{
		return OccupiedAccentColor;
	}

	/** 외곽선 표시 여부 Setter */
	void SetShowOutline(const bool bInShowOutline)
	{
		bShowOutline = bInShowOutline;
	}

	/** 점유 Fill 표시 여부 Setter */
	void SetShowOccupiedFill(const bool bInShowOccupiedFill)
	{
		bShowOccupiedFill = bInShowOccupiedFill;
	}

	/** 점유 Fill 강조 색상 Setter */
	void SetOccupiedAccentColorValue(const FLinearColor& InOccupiedAccentColor)
	{
		OccupiedAccentColor = InOccupiedAccentColor;
	}

public:
	/** 비주얼 상태 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Visual Widget")
	void ResetVisualState();

	/** 비주얼 상태 전체 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Visual Widget")
	void UpdateVisualState(
		bool bInShowOutline,
		bool bInShowOccupiedFill,
		FLinearColor InOccupiedAccentColor);

	/** 외곽선 표시 상태 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Visual Widget")
	void SetOutlineVisible(bool bInShowOutline);

	/** 점유 Fill 표시 상태 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Visual Widget")
	void SetOccupiedFillVisible(bool bInShowOccupiedFill);

	/** 점유 Fill 강조 색상 갱신 요청 */
	UFUNCTION(BlueprintCallable, Category="Inventory Cell Visual Widget")
	void SetOccupiedAccentColor(FLinearColor InOccupiedAccentColor);

protected:
	/** 현재 상태를 실제 비주얼에 반영 */
	void ApplyVisualState();

public:
protected:
	/** 정적 외곽선 */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Cell Visual Widget")
	TObjectPtr<UBorder> Outline;

	/** 점유 상태 표시 Fill */
	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="Inventory Cell Visual Widget")
	TObjectPtr<UBorder> OccupiedFill;

	/** 외곽선 표시 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Cell Visual Widget")
	bool bShowOutline;

	/** 점유 Fill 표시 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Cell Visual Widget")
	bool bShowOccupiedFill;

	/** 점유 Fill 강조 색상 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Inventory Cell Visual Widget")
	FLinearColor OccupiedAccentColor;

private:
};
