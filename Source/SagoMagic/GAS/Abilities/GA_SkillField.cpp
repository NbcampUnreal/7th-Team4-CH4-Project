#include "GA_SkillField.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "SkillActor/SMASkillField.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UGA_SkillField::UGA_SkillField()
{
	FieldClass = ASMASkillField::StaticClass();
}

void UGA_SkillField::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!Avatar) return;
	
	// 장팍 이펙트 예측 발생
	if (Avatar->IsLocallyControlled() && !Avatar->HasAuthority())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = TargetLocation;
			ASC->AddGameplayCue(SMSkillTag::GameplayCue_Skill_SpawnField_Tick, CueParams);
		}
		
		return;
	}
	
	// 서버에서 장팍액터 스폰
	if (Avatar->HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!World || !FieldClass) return;
		
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
		if (!SpecHandle.IsValid()) return;
		
		FActorSpawnParameters Params;
		Params.Owner = Avatar;
		Params.Instigator = Avatar;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// 서버에 스폰
		ASMASkillField* Field = World->SpawnActor<ASMASkillField>(FieldClass, TargetLocation, FRotator::ZeroRotator, Params);
		
		UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
		if (Field)
		{
			Field->InitField(SpecHandle, Avatar, FieldDuration);
			
			if (SourceASC)
			{
				FGameplayCueParameters CueParams;
				CueParams.Location = TargetLocation;
				CueParams.RawMagnitude = FieldDuration;
				SourceASC->AddGameplayCue(SMSkillTag::GameplayCue_Skill_SpawnField_Tick, CueParams);
			}
		}
	}
}
