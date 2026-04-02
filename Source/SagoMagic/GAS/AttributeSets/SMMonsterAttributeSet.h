#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "SMMonsterAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class SAGOMAGIC_API USMMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
    USMMonsterAttributeSet();

    // --- 기본 속성 예시 ---

    /** HP **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, Health)

    /** 최대 HP **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, MaxHealth)

    /** 공격력 **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, AttackPower)

    /** 방어력  **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, Defense)

    /** 이동속도 **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData MoveSpeed;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, MoveSpeed)

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
