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
    if (!Avatar || !Avatar->HasAuthority()) return;

    UWorld* World = GetWorld();
    if (!World || !ProjectileClass) return;

    // 캐릭터 앞 50cm 에서 발사
    const FVector SpawnLocation = CurrentAimOrigin + CurrentAimDirection * 50.f;

    FActorSpawnParameters Params;
    Params.Owner      = Avatar;
    Params.Instigator = Avatar;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    //에디터에서 설정한 클래스 사용
    ASMASkillProjectile* Proj = World->SpawnActor<ASMASkillProjectile>(ProjectileClass, SpawnLocation, CurrentAimDirection.Rotation(), Params);

    if (Proj)
    {
        Proj->InitProjectile(BaseDamage, RangeCm, CurrentAimDirection,Avatar, Avatar->GetController());
    }
}
