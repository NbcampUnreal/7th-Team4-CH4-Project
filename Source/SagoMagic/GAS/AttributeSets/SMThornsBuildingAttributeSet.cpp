#include "SMThornsBuildingAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Building/SMThornsFenceBuilding.h"

void USMThornsBuildingAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		if (Data.EvaluatedData.Magnitude < 0.f)
		{
			AActor* Attacker = Data.EffectSpec.GetContext().GetInstigator();
			if (Attacker)
			{
				ASMThornsFenceBuilding* ThornsFenceBuilding = Cast<ASMThornsFenceBuilding>(GetOwningActor());
				UAbilitySystemComponent* AttackerASC = 
					UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);
				
				if (ThornsFenceBuilding && AttackerASC)
					ThornsFenceBuilding->ApplyThornsDamage(GetOwningAbilitySystemComponent(), AttackerASC);
			}
		}
	}
	
	Super::PostGameplayEffectExecute(Data);
}
