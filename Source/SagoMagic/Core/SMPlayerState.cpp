// SMPlayerState.cpp


#include "SMPlayerState.h"

#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

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

void ASMPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	
	ASMPlayerState* NewPS = Cast<ASMPlayerState>(PlayerState);
	if (IsValid(NewPS) == false) return;
	
	NewPS->bIsHost = bIsHost;
	NewPS->bIsReady = bIsReady;
	NewPS->SetPlayerName(GetPlayerName());
}

void ASMPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASMPlayerState, bIsHost);
	DOREPLIFETIME(ASMPlayerState, bIsReady);
}