#include "SMAbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemBlueprintLibrary.h"

USMAbilitySystemComponent::USMAbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USMAbilitySystemComponent::ApplySkillDamage(AActor* TargetActor, float DamageAmount, AActor* InstigatorActor,
    AController* InstigatorController, TSubclassOf<UGameplayEffect> DamageEffectClass)
{
    if (!TargetActor || !InstigatorActor) return;

    // GAS 경로 — 몬스터에 ASC 있으면 GE 적용
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorActor);

    if (TargetASC && SourceASC && DamageEffectClass)
    {
        FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
        ContextHandle.AddInstigator(InstigatorActor, InstigatorController);

        FGameplayEffectSpecHandle SpecHandle =
            SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);

        if (SpecHandle.IsValid())
        {
            // SetByCaller로 데미지 수치 주입 (BP_GE_EnergyBall의 Data.Damage.Amount)
            SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.Damage.Amount")), -DamageAmount);
            SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
        }
        return;
    }

    // 폴백 — ASC 없는 몬스터
    UGameplayStatics::ApplyDamage(TargetActor, DamageAmount,
        InstigatorController, InstigatorActor, nullptr);
}

void USMAbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USMAbilitySystemComponent::TickComponent(float DeltaTime,
                                              ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

