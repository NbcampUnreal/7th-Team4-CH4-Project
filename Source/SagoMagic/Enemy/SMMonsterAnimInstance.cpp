#include "Enemy/SMMonsterAnimInstance.h"

void USMMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!OwnerMonster)
    {
        OwnerMonster = Cast<ASMMonsterBase>(TryGetPawnOwner());
        if (!OwnerMonster) return;
    }

    // 이동 속도 → 블렌드스페이스용
    Speed = OwnerMonster->GetVelocity().Size();

    // HP 기반 사망 여부
    if (OwnerMonster->MonsterAttributeSet)
    {
        bIsDead = OwnerMonster->MonsterAttributeSet->GetHealth() <= 0.f;
    }
}