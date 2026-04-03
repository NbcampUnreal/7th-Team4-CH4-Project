// SMPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "SMPlayerState.generated.h"

class USMPlayerAttributeSet;

/**
 * 플레이어의 정보를 저장할 클래스
 * 
 * ASC와 AttributeSet 소유
 */
UCLASS()
class SAGOMAGIC_API ASMPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ASMPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	FORCEINLINE USMAbilitySystemComponent* GetSMAbilitySystemComponent() const { return SMAbilitySystemComponent; }
	
	FORCEINLINE USMPlayerAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
protected:
	/** ASC */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USMAbilitySystemComponent> SMAbilitySystemComponent;
	
	/** Player Attribute Set */
	UPROPERTY()
	TObjectPtr<USMPlayerAttributeSet> AttributeSet;
};
