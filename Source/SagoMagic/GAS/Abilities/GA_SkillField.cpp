#include "GA_SkillField.h"
#include "AbilitySystemComponent.h"
#include "SkillActor/SMASkillField.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/Character/SMSkillTag.h"

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

	if (!Avatar->HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World || !FieldClass) return;


	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
	if (!SpecHandle.IsValid()) return;

	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Avatar;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 서버에 스폰
	ASMASkillField* Field = World->SpawnActor<ASMASkillField>(
		FieldClass, TargetLocation, FRotator::ZeroRotator, Params);

	if (IsValid(Field))
	{
		const bool bEnablePull = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_Field_Pull);
		const bool bEnableSlow = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_Field_Slow);
		Field->InitField(SpecHandle, Avatar, FieldDuration, RangeCm, bEnablePull, bEnableSlow);
	}
}
