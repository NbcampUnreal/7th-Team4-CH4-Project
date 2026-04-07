#include "GA_Projectile.h"
#include "SkillActor/SMASkillProjectile.h"


UGA_Projectile::UGA_Projectile()
{
    ProjectileClass = ASMASkillProjectile::StaticClass();
}

void UGA_Projectile::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
    APawn* Avatar = ActorInfo && ActorInfo->AvatarActor.IsValid()
        ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
    if (!Avatar || !Avatar->HasAuthority())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !ProjectileClass)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
    if (!SpecHandle.IsValid())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    // 캐릭터 앞 30cm 에서 발사
    const FVector SpawnLocation = CurrentAimOrigin + CurrentAimDirection * 30.f;

    FActorSpawnParameters Params;
    Params.Owner = Avatar;
    Params.Instigator = Avatar;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASMASkillProjectile* Proj = World->SpawnActor<ASMASkillProjectile>(
        ProjectileClass, SpawnLocation, CurrentAimDirection.Rotation(), Params);

    if (Proj)
    {
        Proj->InitProjectile(SpecHandle, RangeCm, CurrentAimDirection, Avatar);
    }

    EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}
