#pragma once

#include "CoreMinimal.h"
#include "SMFenceBuilding.h"
#include "SMThornsFenceBuilding.generated.h"

struct FActiveGameplayEffectHandle;
class UGameplayEffect;

UCLASS()
class SAGOMAGIC_API ASMThornsFenceBuilding : public ASMFenceBuilding
{
	GENERATED_BODY()
public:
	ASMThornsFenceBuilding();
	void ApplyThornsDamage(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);
	
	virtual void OnDamageReceived(AActor* Attacker, float Amount) override;
public:
	UPROPERTY(EditDefaultsOnly, Category = "Thorns")
	TSubclassOf<UGameplayEffect> ThornsEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Thorns")
	float ThornsDamage = 5.f;
};
