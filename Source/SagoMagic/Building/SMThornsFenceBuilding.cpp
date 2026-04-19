#include "SMThornsFenceBuilding.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AttributeSets/SMThornsBuildingAttributeSet.h"

ASMThornsFenceBuilding::ASMThornsFenceBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASMThornsFenceBuilding::ApplyThornsDamage(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
	if (!ThornsEffectClass || !SourceASC || !TargetASC) return;
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(ThornsEffectClass, 1.f, Context);
	if (!Spec.IsValid()) return;
	
	Spec.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Damage.Amount"), -ThornsDamage);
	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}

void ASMThornsFenceBuilding::OnDamageReceived(AActor* Attacker, float Amount)
{
	if (!HasAuthority() || !Attacker || !ThornsEffectClass) return;
	
	UAbilitySystemComponent* AttackerASC = 
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Attacker);
	
	if (!AttackerASC) return;
	
	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	
	FGameplayEffectSpecHandle Spec = 
		AbilitySystemComponent->MakeOutgoingSpec(ThornsEffectClass, 1.f, Context);
	if (!Spec.IsValid()) return;
	
	Spec.Data->SetSetByCallerMagnitude(
		FGameplayTag::RequestGameplayTag("Data.Damage.Amount"), -ThornsDamage);
	AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), AttackerASC);
}
