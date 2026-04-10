#include "Enemy/SMMonsterAnimInstance.h"
#include "AbilitySystemComponent.h"   
 
#include "SMMonsterBase.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"

//FGameplayTag USMMonsterAnimInstance::AttackingTag;

void USMMonsterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // 게임 시작 시 한 번만 태그 캐싱
    AttackingTag = FGameplayTag::RequestGameplayTag(FName("Enemy.Attacking"));
}

void USMMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerMonster)
    {
        OwnerMonster = Cast<ASMMonsterBase>(TryGetPawnOwner());
        if (!OwnerMonster) return;
    }

    Speed = OwnerMonster->GetVelocity().Size();

    if (OwnerMonster->MonsterAttributeSet)
    {
        bIsDead = OwnerMonster->MonsterAttributeSet->GetHealth() <= 0.f;
    }

    //: ASC 한 번만 캐싱
    if (!CachedASC)
    {
        CachedASC = OwnerMonster->GetAbilitySystemComponent();
    }

    // 태그 유무로 bIsAttacking 결정
    if (CachedASC)
    {
        bIsAttacking = CachedASC->HasMatchingGameplayTag(AttackingTag);
        /*bIsAttacking = CachedASC->HasMatchingGameplayTag(
            FGameplayTag::RequestGameplayTag(FName("Enemy.Attacking"))
        );*/
    }
}