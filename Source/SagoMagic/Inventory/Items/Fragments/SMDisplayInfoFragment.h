#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMDisplayInfoFragment.generated.h"


/**
 * 아이템 표시 정보 Fragment 정의 파일
 *
 * 포함 내용:
 * - 표시 이름
 * - 설명
 * - 강조 색상(인벤토리 내 색상)
 *
 * 역할:
 * - 인벤토리 UI 및 툴팁 표시용 정보 제공
 */

/** 아이템 표시 정보 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMDisplayInfoFragment : public USMItemFragment
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMDisplayInfoFragment()
        : AccentColor(FLinearColor::White)
    {
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

    /** 표시 이름 Setter */
    void SetDisplayName(const FText& InDisplayName)
    {
        DisplayName = InDisplayName;
    }

    /** 설명 Setter */
    void SetDescription(const FText& InDescription)
    {
        Description = InDescription;
    }

    /** 강조 색상 Setter */
    void SetAccentColor(const FLinearColor& InAccentColor)
    {
        AccentColor = InAccentColor;
    }

public:
    /** 표시 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display Info Fragment")
    FText DisplayName;

    /** 설명 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display Info Fragment", meta=(MultiLine=true))
    FText Description;

    /** 강조 색상 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Display Info Fragment")
    FLinearColor AccentColor;
};
