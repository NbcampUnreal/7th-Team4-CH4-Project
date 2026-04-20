#include "GA_Projectile.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Core/DataManager/SMSoundManager.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "SkillActor/SMASkillProjectile.h"


UGA_Projectile::UGA_Projectile()
{
	ProjectileClass = ASMASkillProjectile::StaticClass();
}

void UGA_Projectile::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
{
	APawn* Avatar = ActorInfo && ActorInfo->AvatarActor.IsValid()
		                ? Cast<APawn>(ActorInfo->AvatarActor.Get())
		                : nullptr;

	// 발사 사운드는 로컬 클라이언트에서만 재생
	if (Avatar && Avatar->IsLocallyControlled())
	{
		USMSoundManager* SM = USMSoundManager::Get(this);
		if (IsValid(SM) == true)
		{
			SM->PlaySoundAtLocation(TEXT("ProjectileCast"), Avatar->GetActorLocation());
		}
	}

	if (!Avatar || !Avatar->HasAuthority()) return;


	UWorld* World = GetWorld();
	if (!World || !ProjectileClass) return;

	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
	if (!SpecHandle.IsValid()) return;

	// 스태프끝에서 발사
	FName MuzzleSocketName = FName("Staff_Tip");
	// 소캣 없으면 캐릭터 앞 30cm
	FVector SpawnLocation = CurrentAimOrigin + CurrentAimDirection * 50.f;

	USkeletalMeshComponent* CharacterMesh = Avatar->GetComponentByClass<USkeletalMeshComponent>();
	if (CharacterMesh)
	{
		// 애니메이션 포즈에 맞춰 서버 뼈대 좌표 갱신
		CharacterMesh->RefreshBoneTransforms();
		if (CharacterMesh->DoesSocketExist(MuzzleSocketName))
		{
			SpawnLocation = CharacterMesh->GetSocketLocation(MuzzleSocketName);
		}
	}

	FVector FinalDirection = AimDirection;
	float DistanceToTarget = FVector::Dist2D(Avatar->GetActorLocation(), TargetLocation);

	if (DistanceToTarget > 20.0f)
	{
		FinalDirection = (TargetLocation - SpawnLocation).GetSafeNormal2D();
	}
	
	bEnableHoming = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_Projectile_Homing);
	
	//멀티샷 태그 소지시 발사체 개수 3개 추후 기획 방향성에 따라서 수정가능
	if (SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_Projectile_Multishot))
	{
		ProjectileCount = 3;
	}
	else
	{
		ProjectileCount = 1;
	}

	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Avatar;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < ProjectileCount; i++)
	{
		//projectile 사이의 간격을 SpreadAngle만큼 띄움 ex) SpreadAngle = 15 -> -15 0 15
		float AngleOffset = (i - (ProjectileCount - 1) * 0.5f) * SpreadAngle;
		FVector ShotDirection = FinalDirection.RotateAngleAxis(AngleOffset, FVector::UpVector);
		
		ASMASkillProjectile* Proj = World->SpawnActor<ASMASkillProjectile>(
		ProjectileClass,
		SpawnLocation,
		ShotDirection.Rotation(),
		Params);

		if (Proj)
		{
			Proj->InitProjectile(SpecHandle, RangeCm, ShotDirection, Avatar,bEnableHoming);
		}
	}
}
