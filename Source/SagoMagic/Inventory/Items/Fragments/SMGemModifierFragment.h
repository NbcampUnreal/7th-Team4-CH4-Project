#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMGemModifierFragment.generated.h"


/**
 * 젬 효과 Fragment 정의 파일
 *
 * 포함 내용:
 * - 효과 종류
 * - 효과 수치
 * - 적용 우선순위
 * - 장착 대상 요구 태그
 * - 장착 대상 차단 태그
 *
 * 역할:
 * - 젬 장착 시 적용할 효과와 장착 가능 조건 제공
 *
 * RequiredTargetTags : 해당 태그를 가진 스킬에만 장착 가능(하나도 없어야 자유장착 가능, '전부 만족' 조건을 사용해 설정된 모든 태그를 만족해야 장착가능)
 * BlockedTargetTags : 해당 태그를 가진 스킬에는 장착 불가능(하나도 없어야 자유장착 가능, '일부 만족' 조건을 사용해 하나라도 해당되면 장착 불가능)
 */

/** 젬 효과 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMGemModifierFragment : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMGemModifierFragment()
		: ModifierType(ESMGemModifierType::None)
		  , ModifierValue(0)
		  , ModifierPriority(0)
	{
	}

	/** 효과 종류 Getter */
	ESMGemModifierType GetModifierType() const
	{
		return ModifierType;
	}

	/** 효과 수치 Getter */
	int32 GetModifierValue() const
	{
		return ModifierValue;
	}

	/** 적용 우선순위 Getter */
	int32 GetModifierPriority() const
	{
		return ModifierPriority;
	}

	/** 요구 대상 태그 Getter */
	const FGameplayTagContainer& GetRequiredTargetTags() const
	{
		return RequiredTargetTags;
	}

	/** 차단 대상 태그 Getter */
	const FGameplayTagContainer& GetBlockedTargetTags() const
	{
		return BlockedTargetTags;
	}

	/** 효과 종류 Setter */
	void SetModifierType(const ESMGemModifierType InModifierType)
	{
		ModifierType = InModifierType;
	}

	/** 효과 수치 Setter */
	void SetModifierValue(const int32 InModifierValue)
	{
		ModifierValue = InModifierValue;
	}

	/** 적용 우선순위 Setter */
	void SetModifierPriority(const int32 InModifierPriority)
	{
		ModifierPriority = InModifierPriority;
	}

	/** 요구 대상 태그 Setter */
	void SetRequiredTargetTags(const FGameplayTagContainer& InRequiredTargetTags)
	{
		RequiredTargetTags = InRequiredTargetTags;
	}

	/** 차단 대상 태그 Setter */
	void SetBlockedTargetTags(const FGameplayTagContainer& InBlockedTargetTags)
	{
		BlockedTargetTags = InBlockedTargetTags;
	}

public:
	/** 효과 종류 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	ESMGemModifierType ModifierType;

	/** 효과 수치 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	int32 ModifierValue;

	/** 적용 우선순위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	int32 ModifierPriority;

	/** 장착 대상 스킬 요구 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer RequiredTargetTags;

	/** 장착 대상 스킬 차단 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer BlockedTargetTags;
};
