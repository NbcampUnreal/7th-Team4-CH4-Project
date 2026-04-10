#include "SMBaseCampActor.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "Core/SMGameMode.h"

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

float ASMBaseCampActor::GetCurrentHealth() const
{
	if (AttributeSet)
		return AttributeSet->GetHealth();
	return 0.f;
}

void ASMBaseCampActor::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this,this);

	if (HasAuthority())
	{
		if (ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>())
		{
			GM->RegisterBaseCamp(this);
		}
	}
}

