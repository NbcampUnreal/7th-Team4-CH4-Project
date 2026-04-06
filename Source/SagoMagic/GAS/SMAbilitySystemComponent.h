#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SMAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:

    USMAbilitySystemComponent();
    
    static void ApplySkillDamage(AActor* TargetActor, float DamageAmount,AActor* InstigatorActor, AController* InstigatorController,TSubclassOf<UGameplayEffect> DamageEffectClass = nullptr);

protected:
    
    virtual void BeginPlay() override;

public:

    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
};
