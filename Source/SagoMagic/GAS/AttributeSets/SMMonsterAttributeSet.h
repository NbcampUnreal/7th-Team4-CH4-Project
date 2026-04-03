#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "SagoMagic.h"
#include "SMMonsterAttributeSet.generated.h"


UCLASS()
class SAGOMAGIC_API USMMonsterAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
    USMMonsterAttributeSet();

    /** HP **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health)
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, Health)

    /** 최대 HP **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth)
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, MaxHealth)

    /** 공격력 **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_AttackPower)
    FGameplayAttributeData AttackPower;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, AttackPower)

    /** 방어력  **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Defense)
    FGameplayAttributeData Defense;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, Defense)

    /** 이동속도 **/
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MoveSpeed)
    FGameplayAttributeData MoveSpeed;
    ATTRIBUTE_ACCESSORS(USMMonsterAttributeSet, MoveSpeed)


     UFUNCTION()
    virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
    UFUNCTION() 
    virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
    UFUNCTION() 
    virtual void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
    UFUNCTION()
    virtual void OnRep_Defense(const FGameplayAttributeData& OldDefense);
    UFUNCTION()
    virtual void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);

    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
