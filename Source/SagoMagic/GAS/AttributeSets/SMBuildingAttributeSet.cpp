#include "SMBuildingAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Building/SMBaseBuilding.h"
#include "Net/UnrealNetwork.h"

USMBuildingAttributeSet::USMBuildingAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void USMBuildingAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(USMBuildingAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMBuildingAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void USMBuildingAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}

void USMBuildingAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float Clamped = FMath::Clamp(GetHealth(), 0.f, GetMaxHealth());
		SetHealth(Clamped);

		if (Clamped <= 0.f)
		{
			if (ASMBaseBuilding* Building = Cast<ASMBaseBuilding>(GetOwningActor()))
			{
				Building->HandleDestruction();
			}
		}
	}
}

void USMBuildingAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMBuildingAttributeSet, Health, OldHealth);
}

void USMBuildingAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMBuildingAttributeSet, MaxHealth, OldMaxHealth);
}
