#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Blueprint/UserWidget.h"
#include "SMInteractionWorldInfoWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class UBorder;
class UWidget;


/**
 * 월드 상호작용 정보 표시 데이터 Struct 정의
 *
 * 포함 내용:
 * - 표시 이름
 * - 설명
 * - 요약 스탯 텍스트
 * - 강조 색상
 * - 비트마스크 모양 데이터
 * - 표시 회전값
 *
 * 역할:
 * - 상호작용 대상이 월드 위젯에 전달할 표시 데이터를 하나로 묶어 관리
 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMInteractionWorldInfoData
{
	GENERATED_BODY()

public:
	FSMInteractionWorldInfoData()
		: AccentColor(FLinearColor::White)
		  , ShapeRotation(ESMGridRotation::Rot0)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info", meta=(MultiLine=true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info", meta=(MultiLine=true))
	FText SummaryText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info")
	FLinearColor AccentColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info")
	FSMGridMaskData ShapeMask;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction World Info")
	ESMGridRotation ShapeRotation;
};


/**
 * 월드 상호작용 정보 위젯 정의 파일
 *
 * 포함 내용:
 * - 표시 이름
 * - 설명
 * - 요약 스탯
 * - 강조 색상
 * - 비트마스크 기반 모양 프리뷰
 *
 * 역할:
 * - 월드 상호작용 대상 위에 뜨는 정보 패널을 구성하고
 *   비트마스크 모양을 Canvas 기반으로 렌더링
 */
UCLASS()
class SAGOMAGIC_API USMInteractionWorldInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USMInteractionWorldInfoWidget(const FObjectInitializer& ObjectInitializer);

	/** 현재 표시 이름 Getter */
	const FText& GetDisplayName() const
	{
		return DisplayName;
	}

	/** 현재 설명 Getter */
	const FText& GetDescription() const
	{
		return Description;
	}

	/** 현재 요약 스탯 Getter */
	const FText& GetSummaryText() const
	{
		return SummaryText;
	}

	/** 현재 강조 색상 Getter */
	const FLinearColor& GetAccentColor() const
	{
		return AccentColor;
	}

	/** 현재 Shape 마스크 Getter */
	const FSMGridMaskData& GetShapeMask() const
	{
		return ShapeMask;
	}

	/** 현재 Shape 회전값 Getter */
	ESMGridRotation GetShapeRotation() const
	{
		return ShapeRotation;
	}

	/** 현재 표시 여부 Getter */
	bool HasDisplayData() const
	{
		return bHasDisplayData;
	}

public:
	/** 월드 상호작용 정보 표시 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction World Info Widget")
	void ShowInteractionInfo(const FSMInteractionWorldInfoData& InDisplayData);

	/** 월드 상호작용 정보 숨김 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction World Info Widget")
	void HideInteractionInfo();

protected:
	/** 현재 표시 데이터를 실제 위젯에 반영 */
	void ApplyDisplayDataToWidget();

	/** Shape 프리뷰 셀 재구성 */
	void RebuildShapePreview();

	/** 회전 반영 후 점유 셀 위치 계산 */
	bool CalculatePreviewCellPosition(int32 InLocalX, int32 InLocalY, int32& OutColumn, int32& OutRow) const;

	/** 현재 Shape 기준 점유 셀 수집 */
	void BuildOccupiedCells(TArray<FIntPoint>& OutOccupiedCells) const;

	/** 점유 셀 Bounds 계산 */
	bool CalculateOccupiedCellBounds(const TArray<FIntPoint>& InOccupiedCells,
	                                 int32& OutMinX,
	                                 int32& OutMinY,
	                                 int32& OutMaxX,
	                                 int32& OutMaxY) const;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	FText SummaryText;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	FLinearColor AccentColor;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	FSMGridMaskData ShapeMask;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	ESMGridRotation ShapeRotation;

	UPROPERTY(BlueprintReadOnly, Category="Interaction World Info Widget")
	bool bHasDisplayData;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UCanvasPanel> ShapePreviewCanvas;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UWidget> InfoRoot;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UTextBlock> DisplayNameTextBlock;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UTextBlock> DescriptionTextBlock;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UTextBlock> SummaryTextBlock;

	UPROPERTY(meta=(BindWidget), BlueprintReadOnly, Category="Interaction World Info Widget")
	TObjectPtr<UBorder> AccentColorBorder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction World Info Widget")
	FVector2D PreviewAreaSize = FVector2D(96.0f, 96.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction World Info Widget")
	float MinPreviewCellSize = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction World Info Widget")
	float MaxPreviewCellSize = 32.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction World Info Widget")
	float PreviewCellPadding = 2.0f;
};
