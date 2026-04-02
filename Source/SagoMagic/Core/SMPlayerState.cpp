// SMPlayerState.cpp


#include "SMPlayerState.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"

ASMPlayerState::ASMPlayerState()
{
	SMAbilitySystemComponent = CreateDefaultSubobject<USMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	SMAbilitySystemComponent->SetIsReplicated(true);
	SMAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);
	
	AttributeSet = CreateDefaultSubobject<USMPlayerAttributeSet>(TEXT("AttributeSet"));
	
	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ASMPlayerState::GetAbilitySystemComponent() const
{
	return SMAbilitySystemComponent;
}
