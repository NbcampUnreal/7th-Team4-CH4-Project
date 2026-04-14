#include "GA_Projectile.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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

	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Avatar;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASMASkillProjectile* Proj = World->SpawnActor<ASMASkillProjectile>(
		ProjectileClass,
		SpawnLocation,
		FinalDirection.Rotation(),
		Params);

	if (Proj)
	{
		Proj->InitProjectile(SpecHandle, RangeCm, FinalDirection, Avatar);
	}
}
