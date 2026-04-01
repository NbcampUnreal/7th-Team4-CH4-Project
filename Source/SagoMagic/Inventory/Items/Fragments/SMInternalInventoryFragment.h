#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMInternalInventoryFragment.generated.h"


/**
 * 내부 인벤토리 Fragment 정의 파일
 *
 * 포함 내용:
 * - 내부 인벤토리 마스크
 * - 젬 장착 허용 여부
 * - 동일 이름 빈 스킬 장착 허용 여부
 *
 * 역할:
 * - 스킬 아이템이 보유하는 내부 인벤토리 규칙 제공
 */

/** 내부 인벤토리 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMInternalInventoryFragment : public USMItemFragment
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMInternalInventoryFragment()
        : bAllowGems(true)
        , bAllowSameNamedEmptySkill(true)
    {
    }

    /** 내부 인벤토리 마스크 Getter */
    const FSMGridMaskData& GetInternalMask() const
    {
        return InternalMask;
    }

    /** 젬 장착 허용 여부 Getter */
    bool IsGemAllowed() const
    {
        return bAllowGems;
    }

    /** 동일 이름 빈 스킬 장착 허용 여부 Getter */
    bool IsSameNamedEmptySkillAllowed() const
    {
        return bAllowSameNamedEmptySkill;
    }

    /** 내부 인벤토리 마스크 Setter */
    void SetInternalMask(const FSMGridMaskData& InInternalMask)
    {
        InternalMask = InInternalMask;
    }

    /** 젬 장착 허용 여부 Setter */
    void SetAllowGems(const bool bInAllowGems)
    {
        bAllowGems = bInAllowGems;
    }

    /** 동일 이름 빈 스킬 장착 허용 여부 Setter */
    void SetAllowSameNamedEmptySkill(const bool bInAllowSameNamedEmptySkill)
    {
        bAllowSameNamedEmptySkill = bInAllowSameNamedEmptySkill;
    }

public:
    /** 내부 인벤토리 마스크 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Internal Inventory Fragment")
    FSMGridMaskData InternalMask;

    /** 젬 장착 허용 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Internal Inventory Fragment")
    bool bAllowGems;

    /** 동일 이름 빈 스킬 장착 허용 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Internal Inventory Fragment")
    bool bAllowSameNamedEmptySkill;
};
