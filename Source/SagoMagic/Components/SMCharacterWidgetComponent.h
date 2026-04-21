#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GAS/AttributeSets/SMAttributeSetMacros.h"
#include "SMCharacterWidgetComponent.generated.h"


class ASMGoldFloatingText;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMCharacterWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	USMCharacterWidgetComponent();
	
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<ASMGoldFloatingText> GoldTextClass;
	
	FDelegateHandle GoldChangeHandle;
public:
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	void OnGoldChanged(const FOnAttributeChangeData& Data);
	void SpawnGoldFloatingText(float GoldDelta);
};
