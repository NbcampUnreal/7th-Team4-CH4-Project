//ASMLobbyDummy.cpp

#include "SMLobbyDummy.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "UI/SMEnemyHPBarComponent.h"


ASMLobbyDummy::ASMLobbyDummy()
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->bRunPhysicsWithNoController = true;
	}

	EnemyHPBarComponent = CreateDefaultSubobject<USMEnemyHPBarComponent>(TEXT("SMEnemyHPBar"));
	if (EnemyHPBarComponent)
	{
		EnemyHPBarComponent->SetupAttachment(GetRootComponent());
		EnemyHPBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	}
}


void ASMLobbyDummy::BeginPlay()
{
	Super::BeginPlay();

	if (!MonsterAbilitySystemComponent)
	{
		MonsterAbilitySystemComponent = FindComponentByClass<UAbilitySystemComponent>();
	}

	if (MonsterAbilitySystemComponent)
	{
		MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
		MonsterAbilitySystemComponent->AddLooseGameplayTag(SMGameFlowTag::Enemy);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		if (MovementComponent->MovementMode == MOVE_None)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}
}
