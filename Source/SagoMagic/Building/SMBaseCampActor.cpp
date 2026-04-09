#include "SMBaseCampActor.h"

ASMBaseCampActor::ASMBaseCampActor()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<USMBaseCampAttributeSet>(TEXT("AttributeSet"));
	
	PrimaryActorTick.bCanEverTick = false;
}

UAbilitySystemComponent* ASMBaseCampActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASMBaseCampActor::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this,this);
}

