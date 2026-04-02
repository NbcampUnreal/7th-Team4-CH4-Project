#include "SMGameState.h"

ASMGameState::ASMGameState()
{

}

void ASMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ASMGameState::PostNetInit()
{
    Super::PostNetInit();
}
