#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMAbilityFragment.generated.h"

class UGameplayAbility;
class UGameplayEffect;


/**
 * Ability 참조 Fragment 정의 파일
 *
 * 포함 내용:
 * - Ability 클래스
 * - Ability 입력 태그
 * - Cooldown Effect 클래스
 * - Cost Effect 클래스
 * (이펙트 클래스들은 어빌리티 동작방식에 따라 필요없을수도 있음)
 *
 * 역할:
 * - 스킬 아이템이 실행할 Ability 관련 참조 정보 제공
 */

/** Ability 참조 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMAbilityFragment : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMAbilityFragment() = default;

	/** Ability 클래스 Getter */
	const TSubclassOf<UGameplayAbility>& GetAbilityClass() const
	{
		return AbilityClass;
	}

	/** Ability 입력 태그 Getter */
	const FGameplayTag& GetAbilityInputTag() const
	{
		return AbilityInputTag;
	}

	/** Cooldown Effect 클래스 Getter */
	const TSubclassOf<UGameplayEffect>& GetCooldownEffectClass() const
	{
		return CooldownEffectClass;
	}

	/** Cost Effect 클래스 Getter */
	const TSubclassOf<UGameplayEffect>& GetCostEffectClass() const
	{
		return CostEffectClass;
	}

	/** Ability 클래스 Setter */
	void SetAbilityClass(const TSubclassOf<UGameplayAbility>& InAbilityClass)
	{
		AbilityClass = InAbilityClass;
	}

	/** Ability 입력 태그 Setter */
	void SetAbilityInputTag(const FGameplayTag& InAbilityInputTag)
	{
		AbilityInputTag = InAbilityInputTag;
	}

	/** Cooldown Effect 클래스 Setter */
	void SetCooldownEffectClass(const TSubclassOf<UGameplayEffect>& InCooldownEffectClass)
	{
		CooldownEffectClass = InCooldownEffectClass;
	}

	/** Cost Effect 클래스 Setter */
	void SetCostEffectClass(const TSubclassOf<UGameplayEffect>& InCostEffectClass)
	{
		CostEffectClass = InCostEffectClass;
	}

public:
	/** Ability 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability Fragment")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Ability 입력 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability Fragment")
	FGameplayTag AbilityInputTag;

	/** Cooldown Effect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability Fragment")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	/** Cost Effect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ability Fragment")
	TSubclassOf<UGameplayEffect> CostEffectClass;
};
