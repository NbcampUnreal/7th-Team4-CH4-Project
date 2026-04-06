#include "GA_Projectile.h"
#include "SkillActor/SMASkillProjectile.h"



UGA_Projectile::UGA_Projectile()
{
    ProjectileClass = ASMASkillProjectile::StaticClass();
}

void UGA_Projectile::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // 부모(GA_SkillBase)의 ActivateAbility 호출 - DT 로드, AimData 추출, CommitAbility 처리
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    APawn* Avatar = ActorInfo && ActorInfo->AvatarActor.IsValid()
        ? Cast<APawn>(ActorInfo->AvatarActor.Get()) : nullptr;
    if (!Avatar || !Avatar->HasAuthority())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UWorld* World = GetWorld();
    if (!World || !ProjectileClass)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 캐릭터 앞 30cm 에서 발사
    const FVector SpawnLocation = CurrentAimOrigin + CurrentAimDirection * 30.f;

    FActorSpawnParameters Params;
    Params.Owner = Avatar;
    Params.Instigator = Avatar;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASMASkillProjectile* Proj = World->SpawnActor<ASMASkillProjectile>(ProjectileClass, SpawnLocation, CurrentAimDirection.Rotation(), Params);

    if (Proj)
    {
        Proj->InitProjectile(BaseDamage, RangeCm, CurrentAimDirection, Avatar, Avatar->GetController(), DamageEffectClass);
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
