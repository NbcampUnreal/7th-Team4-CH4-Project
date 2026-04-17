//SMGameplayAbilityUtils.h

#pragma once

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "GameplayTags/Enemy/SMEnemyTag.h"

namespace SMGameplayAbilityUtils
{
	static bool HasTeamTag(AActor* Actor)
	{
		if (IsValid(Actor) == false) return false;

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (IsValid(ASC) == false) return false;

		return ASC->HasMatchingGameplayTag(SMGameFlowTag::Team);
	}

	static bool IsAvailableEnemy(AActor* Actor)
	{
		if (IsValid(Actor) == false) return false;

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (IsValid(ASC) == false) return false;

		if (ASC->HasMatchingGameplayTag(SMEnemyTag::Enemy_State_Death) == true) return false;

		return ASC->HasMatchingGameplayTag(SMGameFlowTag::Enemy);
	}
}