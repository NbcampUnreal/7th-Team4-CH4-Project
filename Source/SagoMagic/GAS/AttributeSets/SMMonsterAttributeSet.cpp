#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USMMonsterAttributeSet::USMMonsterAttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitAttackPower(10.0f);
    InitDefense(5.0f);
    InitMoveSpeed(300.0f);
}

void USMMonsterAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 각 속성들을 네트워크 복제 대상으로 등록 (별도 조건 없이 상시 복제)
	DOREPLIFETIME_CONDITION_NOTIFY(USMMonsterAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMMonsterAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMMonsterAttributeSet, AttackPower, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMMonsterAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMMonsterAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}

void USMMonsterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void USMMonsterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		if (GetHealth() <= 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Monster] HP가 0이 되었습니다. 사망 처리를 시작합니다."));
			// MonsterBase가 바인딩한 HandleDeath()를 호출
			OnMonsterDied.Broadcast();
		}
	}
}

void USMMonsterAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) 
{ 
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMMonsterAttributeSet, Health, OldHealth); 
}
void USMMonsterAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) 
{ 
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMMonsterAttributeSet, MaxHealth, OldMaxHealth); 
}
void USMMonsterAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMMonsterAttributeSet, AttackPower, OldAttackPower);
}
void USMMonsterAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldDefense) 
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMMonsterAttributeSet, Defense, OldDefense);
}
void USMMonsterAttributeSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed) 
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMMonsterAttributeSet, MoveSpeed, OldMoveSpeed);
}