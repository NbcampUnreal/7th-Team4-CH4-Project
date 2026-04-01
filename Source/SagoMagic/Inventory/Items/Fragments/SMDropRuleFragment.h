#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMDropRuleFragment.generated.h"


/**
 * 드랍 정책 Fragment 정의 파일
 *
 * 포함 내용:
 * - 드랍 가능 여부
 * - 삭제 가능 여부
 * - 내부 장착물 포함 드랍 여부
 *
 * 역할:
 * - 아이템 드랍 및 삭제 관련 정책 제공
 *
 * PS. 존재 필요 여부 자체가 애매하긴 한데 일단 두고 추후 삭제할지 논의
 */

/** 드랍 정책 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMDropRuleFragment : public USMItemFragment
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMDropRuleFragment()
        : bCanDrop(true)
        , bCanDestroy(true)
        , bDropWithEmbeddedItems(true)
    {
    }

    /** 드랍 가능 여부 Getter */
    bool CanDrop() const
    {
        return bCanDrop;
    }

    /** 삭제 가능 여부 Getter */
    bool CanDestroy() const
    {
        return bCanDestroy;
    }

    /** 내부 장착물 포함 드랍 여부 Getter */
    bool CanDropWithEmbeddedItems() const
    {
        return bDropWithEmbeddedItems;
    }

    /** 드랍 가능 여부 Setter */
    void SetCanDrop(const bool bInCanDrop)
    {
        bCanDrop = bInCanDrop;
    }

    /** 삭제 가능 여부 Setter */
    void SetCanDestroy(const bool bInCanDestroy)
    {
        bCanDestroy = bInCanDestroy;
    }

    /** 내부 장착물 포함 드랍 여부 Setter */
    void SetDropWithEmbeddedItems(const bool bInDropWithEmbeddedItems)
    {
        bDropWithEmbeddedItems = bInDropWithEmbeddedItems;
    }

public:
    /** 드랍 가능 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Drop Rule Fragment")
    bool bCanDrop;

    /** 삭제 가능 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Drop Rule Fragment")
    bool bCanDestroy;

    /** 내부 장착물 포함 드랍 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Drop Rule Fragment")
    bool bDropWithEmbeddedItems;
};
