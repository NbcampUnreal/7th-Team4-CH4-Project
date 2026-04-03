// SMPlayerAttributeSet.h

#pragma once

#include "CoreMinimal.h"
#include "SagoMagic.h"
#include "AttributeSet.h"

#include "SMPlayerAttributeSet.generated.h"

/**
 * 플레이어의 스탯을 관리하는 AttributeSet
 */
UCLASS()
class SAGOMAGIC_API USMPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	USMPlayerAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	/** Player Health */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, Health)

	/** Player MaxHealth */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|MaxHealth", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, MaxHealth)

	/** Player Gold */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Gold", ReplicatedUsing = OnRep_Gold)
	FGameplayAttributeData Gold;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, Gold)

	/** Player Gold */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Gold", ReplicatedUsing = OnRep_MaxGold)
	FGameplayAttributeData MaxGold;
	ATTRIBUTE_ACCESSORS(USMPlayerAttributeSet, MaxGold)

	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Gold(const FGameplayAttributeData& OldGold);

	UFUNCTION()
	virtual void OnRep_MaxGold(const FGameplayAttributeData& OldMaxGold);

	// Attribute 변경 전 호출
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// Attribute 변경 후 호출
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
