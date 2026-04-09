#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "SMAttributeSetMacros.h"
#include "SMBaseCampAttributeSet.generated.h"

/**
 * Base Camp 용 AttributeSet
 */
UCLASS()
class SAGOMAGIC_API USMBaseCampAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	USMBaseCampAttributeSet();
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	
	UFUNCTION()
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UFUNCTION()
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	
	ATTRIBUTE_ACCESSORS(USMBaseCampAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(USMBaseCampAttributeSet, MaxHealth);
	
};

