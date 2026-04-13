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
 * - 장착 대상 전체 요구 태그
 * - 장착 대상 일부 요구 태그
 * - 장착 대상 차단 태그
 * - 장착시 스킬 효과 변동을 적용할 태그 컨테이너(추후 확장성 대비용)
 *
 * 역할:
 * - 젬 장착 시 적용할 효과와 장착 가능 조건 제공
 *
 * RequiredAllTargetTags : 해당 태그를 가진 스킬에만 장착 가능(하나도 없으면 조건 없음, 설정된 모든 태그를 만족해야 장착 가능)
 * RequiredAnyTargetTags : 해당 태그를 가진 스킬에만 장착 가능(하나도 없으면 조건 없음, 설정된 태그 중 하나라도 만족해야 장착 가능)
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

	/** 전체 요구 대상 태그 Getter */
	const FGameplayTagContainer& GetRequiredAllTargetTags() const
	{
		return RequiredAllTargetTags;
	}

	/** 일부 요구 대상 태그 Getter */
	const FGameplayTagContainer& GetRequiredAnyTargetTags() const
	{
		return RequiredAnyTargetTags;
	}

	/** 차단 대상 태그 Getter */
	const FGameplayTagContainer& GetBlockedTargetTags() const
	{
		return BlockedTargetTags;
	}

	/** 부여 행동 태그 Getter */
	const FGameplayTagContainer& GetGrantedBehaviorTags() const
	{
		return GrantedBehaviorTags;
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

	/** 전체 요구 대상 태그 Setter */
	void SetRequiredAllTargetTags(const FGameplayTagContainer& InRequiredAllTargetTags)
	{
		RequiredAllTargetTags = InRequiredAllTargetTags;
	}

	/** 일부 요구 대상 태그 Setter */
	void SetRequiredAnyTargetTags(const FGameplayTagContainer& InRequiredAnyTargetTags)
	{
		RequiredAnyTargetTags = InRequiredAnyTargetTags;
	}

	/** 차단 대상 태그 Setter */
	void SetBlockedTargetTags(const FGameplayTagContainer& InBlockedTargetTags)
	{
		BlockedTargetTags = InBlockedTargetTags;
	}

	/** 부여 행동 태그 Setter */
	void SetGrantedBehaviorTags(const FGameplayTagContainer& InGrantedBehaviorTags)
	{
		GrantedBehaviorTags = InGrantedBehaviorTags;
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

	/** 장착 대상 스킬 전체 요구 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer RequiredAllTargetTags;

	/** 장착 대상 스킬 일부 요구 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer RequiredAnyTargetTags;

	/** 장착 대상 스킬 차단 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer BlockedTargetTags;

	/** 장착 시 스킬 요약에 추가할 특수 동작 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Gem Modifier Fragment")
	FGameplayTagContainer GrantedBehaviorTags;
};
