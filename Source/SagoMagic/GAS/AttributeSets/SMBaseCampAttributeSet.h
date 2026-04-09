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
	UPROPERTY(BlueprintReadOnly, Category = "Attribute", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	
	ATTRIBUTE_ACCESSORS(USMBaseCampAttributeSet, Health);
	
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);
};
