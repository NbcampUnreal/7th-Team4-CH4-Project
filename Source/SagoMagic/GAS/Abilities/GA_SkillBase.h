#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "GA_SkillBase.generated.h"

class UGameplayEffect;
struct FSMSkillData;

UCLASS()
class SAGOMAGIC_API UGA_SkillBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_SkillBase();

	//어빌리티 인터페이스

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	                           const FGameplayAbilityActorInfo* ActorInfo,
	                           const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** 현재 Prediction Key를 반환하고 더 많은? 예측에 유효한지 Vaild 한다 - 디버깅에 사용 */
	UFUNCTION(BlueprintCallable, Category = "SagoMagic|Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	/** 현재 Prediction Key가 추가 예측에 유효한지 -> Prediction Key가 여전히 사용 가능한지 여부 확인 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SagoMagic|Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

protected:
	//스킬효과 서버
	virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo){}
	
	// 마우스 정보
	void ExtractAimData(const FGameplayAbilityActorInfo* ActorInfo);
	
	//OnSkillEffect 내에서 호출. 유효하지 않으면 Invalid Handle 반환.
	FGameplayEffectSpecHandle MakeDamageSpec(const FGameplayAbilityActorInfo* ActorInfo) const;

	/** 쿨다운중 쿨다운 재발동 차단용 ex) Cooldown.Skill.Projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
	FGameplayTag CooldownTag;

	/** DT_SkillData에서 스킬 행 참조 - 에디터에서 테이블과 행 이름 지정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Stats")
	FDataTableRowHandle SkillStatRow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// ActivateAbility 시 DT에서 읽어서 채워지는 수치
	float BaseDamage = 0.f;
	float RangeCm = 0.f;
	float CooldownSeconds = 0.f;

	FVector CurrentAimOrigin = FVector::ZeroVector;
	FVector CurrentAimDirection = FVector::ForwardVector;

private:
	// DT에서 스킬 수치 로드 ActivateAbility 시작 시 호출
	bool LoadSkillStats();
};
