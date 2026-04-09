#include "SMBaseCampActor.h"

ASMBaseCampActor::ASMBaseCampActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<USMBaseCampAttributeSet>(TEXT("AttributeSet"));
	
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

