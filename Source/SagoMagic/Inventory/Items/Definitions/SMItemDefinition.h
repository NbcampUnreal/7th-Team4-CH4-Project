#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMItemDefinition.generated.h"


/**
 * 공통 아이템 정의 DataAsset 클래스 정의 파일
 *
 * 포함 내용:
 * - 아이템 내부 이름
 * - 아이템 태그
 * - Fragment 배열
 * - Fragment 조회 함수
 *
 * 역할:
 * - 젬, 스킬, 기타 아이템의 공통 정적 정의 제공
 */

/** 공통 아이템 정의 DataAsset 클래스 */
UCLASS(BlueprintType)
class SAGOMAGIC_API USMItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMItemDefinition() = default;

    /** 내부 이름 Getter */
    const FName& GetInternalName() const
    {
        return InternalName;
    }

    /** 아이템 태그 Getter */
    const FGameplayTagContainer& GetItemTags() const
    {
        return ItemTags;
    }

    /** Fragment 배열 Getter */
    const TArray<TObjectPtr<USMItemFragment>>& GetFragments() const
    {
        return Fragments;
    }

    /** 내부 이름 Setter */
    void SetInternalName(const FName& InInternalName)
    {
        InternalName = InInternalName;
    }

    /** 아이템 태그 Setter */
    void SetItemTags(const FGameplayTagContainer& InItemTags)
    {
        ItemTags = InItemTags;
    }

    /** Fragment 배열 Setter */
    void SetFragments(const TArray<TObjectPtr<USMItemFragment>>& InFragments)
    {
        Fragments = InFragments;
    }

public:
    /** 특정 타입 Fragment 조회 템플릿 함수 */
    template <typename T>
    const T* FindFragmentByClass() const
    {
        for (const TObjectPtr<USMItemFragment>& Fragment : Fragments)
        {
            if (const T* CastedFragment = Cast<T>(Fragment))
            {
                return CastedFragment;
            }
        }

        return nullptr;
    }

    /** 특정 타입 Fragment 존재 여부 검사 템플릿 함수 */
    template <typename T>
    bool HasFragmentByClass() const
    {
        return FindFragmentByClass<T>() != nullptr;
    }

public:
    /** 아이템 내부 식별 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Definition")
    FName InternalName;

    /** 아이템 태그 목록 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Definition")
    FGameplayTagContainer ItemTags;

    /** 아이템 기능 조각 배열 */
    UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category="Item Definition")
    TArray<TObjectPtr<USMItemFragment>> Fragments;
};
