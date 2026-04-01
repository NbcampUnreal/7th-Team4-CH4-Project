#include "SMGameState.h"
#include "Net/UnrealNetwork.h"

ASMGameState::ASMGameState()
{

}

UAbilitySystemComponent* ASMGameState::GetAbilitySystemComponent() const
{
    return nullptr;
}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASMGameState::PostNetInit()
{
    Super::PostNetInit();
}
