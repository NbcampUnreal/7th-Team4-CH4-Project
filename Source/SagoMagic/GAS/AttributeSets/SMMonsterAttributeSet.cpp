#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "GameplayEffectExtension.h"

USMMonsterAttributeSet::USMMonsterAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitAttackPower(10.0f);
    InitDefense(5.0f);
    InitMoveSpeed(300.0f);
}

void USMMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);

    // 체력이 MaxHealth를 넘지 않도록 제한
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
}

void USMMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    // 이펙트 적용 후 최종 수치 보정
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

        if (GetHealth() <= 0.0f)
        {
            // TODO: 몬스터 사망 로직 트리거
            UE_LOG(LogTemp, Warning, TEXT("Monster is Dead!"));
        }
    }
}
