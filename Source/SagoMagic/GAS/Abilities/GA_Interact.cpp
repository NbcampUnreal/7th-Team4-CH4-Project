// GA_Interact.cpp


#include "GA_Interact.h"

#include "Character/SMInteractionScannerComponent.h"
#include "Character/SMPlayerCharacter.h"
#include "Components/SMInteractionTargetComponent.h"
#include "GameplayTags/Character/SMCharacterTag.h"
#include "GameplayTags/Character/SMSkillTag.h"

UGA_Interact::UGA_Interact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(SMCharacterTag::Ability_Default_Interact));

	ActivationBlockedTags.AddTag(SMCharacterTag::State_Attacking);

	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Interact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ASMPlayerCharacter* AvatarCharacter = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(AvatarCharacter))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	USMInteractionScannerComponent* Scanner = AvatarCharacter->GetInteractionScanner();
	if (!Scanner)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	USMInteractionTargetComponent* BestTarget = Scanner->GetClosestTarget();
	
	if (IsValid(BestTarget))
	{
		BestTarget->Interact(AvatarCharacter);
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
