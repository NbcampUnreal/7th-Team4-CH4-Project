#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "UGA_SkillBase.generated.h"


UCLASS()
class SAGOMAGIC_API UGA_SkillBase : public UGameplayAbility
{
	GENERATED_BODY()


public:
    UGA_SkillBase();

    //어빌리티 인터페이스

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

    //** 현재 Prediction Key를 반환하고 더 많은? 예측에 유효한지 Vaild 한다 ,Ability Prediction Window 디버깅에 사용 */
    UFUNCTION(BlueprintCallable, Category = "SagoMagic|Ability")
    virtual FString GetCurrentPredictionKeyStatus();

    //** 현재 Prediction Key가 추가 예측에 유효한지 -> Prediction Key가 여전히 사용 가능한지 여부 확인 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SagoMagic|Ability")
    virtual bool IsPredictionKeyValidForMorePrediction() const;

protected:

    //스킬효과 서버
    virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) {}

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
    FGameplayTag SkillTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
    FGameplayTag SkillEffectCueTag;

    //수치-----
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float BaseDamage = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float RangeCm = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float CooldownSeconds= 3.f;

    // TODO:마우스 정보 데이터 가져오기
};
