// AN_SendEvent.cpp


#include "AN_SendEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

UAN_SendEvent::UAN_SendEvent()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 128, 0);
#endif
}

void UAN_SendEvent::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner)) return;
	
	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner)) return;

	FGameplayEventData Payload;
	Payload.Instigator = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
}
