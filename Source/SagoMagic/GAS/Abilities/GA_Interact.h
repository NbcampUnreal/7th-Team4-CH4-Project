// GA_Interact.h

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Interact.generated.h"

/**
 * 플레이어의 DefaultAbility중 하나
 * 다른 액터와 상호작용 하기 위한 어빌리티
 */
UCLASS()
class SAGOMAGIC_API UGA_Interact : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_Interact();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
