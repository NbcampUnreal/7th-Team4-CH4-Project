// SMPlayerAttributeSet.cpp


#include "SMPlayerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

USMPlayerAttributeSet::USMPlayerAttributeSet()
{
	// TODO: GameplayEffect 또는 DataAsset으로 초기화 고려
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitGold(0.0f);
	
	// 참고: AttributeSet특성상 32비트 float로 돼있기 때문에 1600만이 최대
	InitMaxGold(1000000.0f);
}

void USMPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMPlayerAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMPlayerAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMPlayerAttributeSet, Gold, COND_None, REPNOTIFY_Always);
}

void USMPlayerAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMPlayerAttributeSet, Health, OldHealth);
}

void USMPlayerAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMPlayerAttributeSet, MaxHealth, OldMaxHealth);
}

void USMPlayerAttributeSet::OnRep_Gold(const FGameplayAttributeData& OldGold)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMPlayerAttributeSet, Gold, OldGold);
}

void USMPlayerAttributeSet::OnRep_MaxGold(const FGameplayAttributeData& OldMaxGold)
{
}

void USMPlayerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}

	if (Attribute == GetGoldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxGold());
	}
}

void USMPlayerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		// TODO: Health가 0이되면 Death처리 후 UnPosses처리
	}

	if (Data.EvaluatedData.Attribute == GetGoldAttribute())
	{
		SetGold(FMath::Clamp(GetGold(), 0.0f, GetMaxGold()));
	}
}
