#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "SMAttributeSetMacros.h"
#include "SMBuildingAttributeSet.generated.h"

/**
 * 건물 공용 AttributeSet
 */
UCLASS()
class SAGOMAGIC_API USMBuildingAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	USMBuildingAttributeSet();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes",
		ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USMBuildingAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes",
		ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USMBuildingAttributeSet, MaxHealth);
private:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
