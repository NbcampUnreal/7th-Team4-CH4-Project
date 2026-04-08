#include "SMAbilitySystemComponent.h"

USMAbilitySystemComponent::USMAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USMAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
}

