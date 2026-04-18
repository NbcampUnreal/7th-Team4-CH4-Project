//ASMLobbyDummy.cpp

#include "SMLobbyDummy.h"

#include "AbilitySystemComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"


ASMLobbyDummy::ASMLobbyDummy()
{
}


void ASMLobbyDummy::BeginPlay()
{
	if (!MonsterAbilitySystemComponent)
	{
		MonsterAbilitySystemComponent = FindComponentByClass<UAbilitySystemComponent>();
	}

	MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
	MonsterAbilitySystemComponent->AddLooseGameplayTag(SMGameFlowTag::Enemy);
}
