#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "UGA_SkillBase.generated.h"


UCLASS()
class SAGOMAGIC_API UGA_SkillBase : public UGameplayAbility
{
	GENERATED_BODY()
	UGA_SkillBase();

public:

    //** 어빌리티 인터페이스 */

    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;


/*
    // 어빌리티 Handle을 활성화, 모든 RPC를 하나로 묶으려고 시도
    // 특정 상황에서만 발생하는 모든 RPC만 묶음, 한 프레임 내에서 처리
    //ActivateAbility, EndAbility 2개의 RPC 대신 하나의 RPC로 처리 할 수 있음 마취 Bunch TODO로 남겨두지만 추후 사라질 수 있음.
    UFUNCTION(BlueprintCallable, Category = "SagoMagic|Ability")
    virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);
*/

    //** 현재 Prediction Key를 반환하고 더 많은? 예측에 유효한지 Vaild 한다 ,Ability Prediction Window 디버깅에 사용 */
    UFUNCTION(BlueprintCallable, Category = "SagoMagic|Ability")
    virtual FString GetCurrentPredictionKeyStatus();

    //* 현재 Prediction Key가 추가 예측에 유효한지 -> Prediction Key가 여전히 사용 가능한지 여부 확인 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SagoMagic|Ability")
    virtual bool IsPredictionKeyValidForMorePrediction() const;



protected:

    /**ㅅ,킬효과 서버*/
    virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) {}

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
    FGameplayTag SkillTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
    FGameplayTag SkillEffectCueTag;

    //** 수치들*/
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float BaseDamage = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float RangeCm = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SagoMagic|Stats")
    float CooldownSeconds= 3.f;

    // TODO:마우스 정보 데이터 가져오기
};
