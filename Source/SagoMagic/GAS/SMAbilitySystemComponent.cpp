#include "SMAbilitySystemComponent.h"

USMAbilitySystemComponent::USMAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USMAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USMAbilitySystemComponent::TickComponent(float DeltaTime,
                                              ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

