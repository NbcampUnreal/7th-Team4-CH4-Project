// GA_SkillTurret.cpp


#include "GA_SkillTurret.h"

#include "GameplayTags/Character/SMSkillTag.h"
#include "SkillActor/SMSkillTurret.h"

void UGA_SkillTurret::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!Avatar || !Avatar->HasAuthority()) return;

	UWorld* World = GetWorld();
	if (!World || !TurretClass) return;

	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
	if (!SpecHandle.IsValid()) return;

	// 업그레이드는 태그로 레벨 판단
	// SkillUpgradeTags는 GA_SkillBase에서 인벤 로드시 추가
	int32 SkillLevel = 1;
	if (SkillUpgradeTags.HasTagExact(SMSkillTag::Upgrade_Turret_Barrage))
	{
		SkillLevel = 3;
	}
	else if (SkillUpgradeTags.HasTagExact(SMSkillTag::Upgrade_Turret_Splash))
	{
		SkillLevel = 2;
	}

	// 2렙 이상이면 스플래쉬 Spec생성
	FGameplayEffectSpecHandle SplashSpec;
	if (SkillLevel >= 2 && SplashDamageEffectClass)
	{
		SplashSpec = MakeOutgoingGameplayEffectSpec(SplashDamageEffectClass);
		if (SplashSpec.IsValid())
		{
			SplashSpec.Data->SetSetByCallerMagnitude(SMSkillTag::Data_Damage_Amount, -(BaseDamage * 0.5f));
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Avatar;
	SpawnParams.Instigator = Avatar;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASMSkillTurret* Turret = World->SpawnActor<ASMSkillTurret>(
		TurretClass,
		TargetLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (IsValid(Turret))
	{
		Turret->InitTurret(
			SpecHandle,
			SplashSpec,
			Avatar,
			FieldDuration,
			RangeCm,
			TickInterval,
			SkillLevel);
	}
}
