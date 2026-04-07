#include "GA_Projectile.h"
#include "SkillActor/SMASkillProjectile.h"
#include "AbilitySystemComponent.h"


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

    // GA 내부에서 GE Spec 생성 (BaseDamage SetByCaller 주입)
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC || !DamageEffectClass)
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }

    FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
    ContextHandle.AddInstigator(Avatar, Avatar->GetController());

    FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);
    if (!SpecHandle.IsValid())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
        return;
    }
    SpecHandle.Data->SetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag(TEXT("Data.Damage.Amount")), -BaseDamage);

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
        // 완성된 Spec을 투사체에 전달
        Proj->InitProjectile(SpecHandle, RangeCm, CurrentAimDirection, Avatar);
    }

    EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}
