#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "SMItemFragments.generated.h"

class UTexture2D;
class UGameplayAbility;
class UGameplayEffect;
class UStaticMesh;
class UMaterialInterface;


/**
 * 아이템 기능 조각(Fragment) 클래스 정의 파일
 *
 * 포함 내용:
 * - 공통 Fragment 베이스 클래스
 * - 표시 정보 Fragment
 * - 외형 마스크 Fragment
 * - 젬 효과 Fragment
 * - Ability 참조 Fragment
 * - 내부 인벤토리 Fragment
 * - 스킬 성장 Fragment
 * - 드랍 정책 Fragment
 * - 월드 비주얼 Fragment
 *
 * 역할:
 * - ItemDefinition이 조합해서 사용할 기능 단위 데이터 조각 정의
 *
 * RequiredTargetTags : 젬을 장착하기 위해 필요한 대상의 GameplayTag(All Matches 방식으로 작동되며, 비어있어야 조건없이 착용가능. 하나라도 있으면 무조건 태그가 일치해야 함)
 * BlockedTargetTags : 젬의 장착을 금지하기 위해 필요한 대상의 GameplayTag(Any Matches 방식으로 작동되며, 해당 태그가 있는 스킬에만 장착 불가능)
 *
 */

/** 공통 아이템 Fragment 베이스 클래스 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMItemFragment : public UObject
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMItemFragment() = default;
};

/** 아이템 표시 정보 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_DisplayInfo : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_DisplayInfo()
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Display")
	FText DisplayName;

	/** 설명 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Display", meta=(MultiLine=true))
	FText Description;

	/** 강조 색상 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Display")
	FLinearColor AccentColor;
};

/** 아이템 외형 마스크 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_GridShape : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_GridShape() = default;

	/** 외형 마스크 Getter */
	const FSMGridMaskData& GetShapeMask() const
	{
		return ShapeMask;
	}

	/** 외형 마스크 Setter */
	void SetShapeMask(const FSMGridMaskData& InShapeMask)
	{
		ShapeMask = InShapeMask;
	}

public:
	/** 외형 마스크 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Grid Shape")
	FSMGridMaskData ShapeMask;
};

/** 젬 효과 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_GemModifier : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_GemModifier()
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Gem Modifier")
	ESMGemModifierType ModifierType;

	/** 효과 수치 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Gem Modifier")
	int32 ModifierValue;

	/** 적용 우선순위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Gem Modifier")
	int32 ModifierPriority;

	/** 장착 대상 스킬 요구 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Gem Modifier")
	FGameplayTagContainer RequiredTargetTags;

	/** 장착 대상 스킬 차단 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Gem Modifier")
	FGameplayTagContainer BlockedTargetTags;
};

/** Ability 참조 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_Ability : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_Ability() = default;

	/** Ability 클래스 Getter */
	const TSubclassOf<UGameplayAbility>& GetAbilityClass() const
	{
		return AbilityClass;
	}

	/** 입력 태그 Getter */
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

	/** 입력 태그 Setter */
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	/** Ability 입력 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Ability")
	FGameplayTag AbilityInputTag;

	/** Cooldown Effect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Ability")
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	/** Cost Effect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Ability")
	TSubclassOf<UGameplayEffect> CostEffectClass;
};

/** 내부 인벤토리 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_InternalInventory : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_InternalInventory()
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Internal Inventory")
	FSMGridMaskData InternalMask;

	/** 젬 장착 허용 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Internal Inventory")
	bool bAllowGems;

	/** 동일 이름 빈 스킬 장착 허용 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Internal Inventory")
	bool bAllowSameNamedEmptySkill;
};

/** 스킬 성장 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_SkillProgression : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_SkillProgression()
		: BaseLevel(1)
		, bLevelFromEmbeddedSameSkill(true)
		, MaxLevel(99)
	{
	}

	/** 기본 레벨 Getter */
	int32 GetBaseLevel() const
	{
		return BaseLevel;
	}

	/** 동일 이름 스킬 장착 시 레벨 증가 여부 Getter */
	bool IsLevelFromEmbeddedSameSkill() const
	{
		return bLevelFromEmbeddedSameSkill;
	}

	/** 최대 레벨 Getter */
	int32 GetMaxLevel() const
	{
		return MaxLevel;
	}

	/** 기본 레벨 Setter */
	void SetBaseLevel(const int32 InBaseLevel)
	{
		BaseLevel = InBaseLevel;
	}

	/** 동일 이름 스킬 장착 시 레벨 증가 여부 Setter */
	void SetLevelFromEmbeddedSameSkill(const bool bInLevelFromEmbeddedSameSkill)
	{
		bLevelFromEmbeddedSameSkill = bInLevelFromEmbeddedSameSkill;
	}

	/** 최대 레벨 Setter */
	void SetMaxLevel(const int32 InMaxLevel)
	{
		MaxLevel = InMaxLevel;
	}

public:
	/** 기본 레벨 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Skill Progression")
	int32 BaseLevel;

	/** 동일 이름 스킬 장착 시 레벨 증가 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Skill Progression")
	bool bLevelFromEmbeddedSameSkill;

	/** 최대 레벨 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Skill Progression")
	int32 MaxLevel;
};

/** 드랍 정책 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_DropRule : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_DropRule()
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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Drop Rule")
	bool bCanDrop;

	/** 삭제 가능 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Drop Rule")
	bool bCanDestroy;

	/** 내부 장착물 포함 드랍 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|Drop Rule")
	bool bDropWithEmbeddedItems;
};

/** 월드 비주얼 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMIF_WorldVisual : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMIF_WorldVisual()
		: WorldScale(FVector(1.0f, 1.0f, 1.0f))
	{
	}

	/** 월드 메시 Getter */
	const TSoftObjectPtr<UStaticMesh>& GetWorldMesh() const
	{
		return WorldMesh;
	}

	/** 오버라이드 머티리얼 Getter */
	const TSoftObjectPtr<UMaterialInterface>& GetOverrideMaterial() const
	{
		return OverrideMaterial;
	}

	/** 월드 스케일 Getter */
	const FVector& GetWorldScale() const
	{
		return WorldScale;
	}

	/** 월드 메시 Setter */
	void SetWorldMesh(const TSoftObjectPtr<UStaticMesh>& InWorldMesh)
	{
		WorldMesh = InWorldMesh;
	}

	/** 오버라이드 머티리얼 Setter */
	void SetOverrideMaterial(const TSoftObjectPtr<UMaterialInterface>& InOverrideMaterial)
	{
		OverrideMaterial = InOverrideMaterial;
	}

	/** 월드 스케일 Setter */
	void SetWorldScale(const FVector& InWorldScale)
	{
		WorldScale = InWorldScale;
	}

public:
	/** 월드 드랍 메시 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|World Visual")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** 오버라이드 머티리얼 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|World Visual")
	TSoftObjectPtr<UMaterialInterface> OverrideMaterial;

	/** 월드 스케일 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Item Fragment|World Visual")
	FVector WorldScale;
};
