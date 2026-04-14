#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterRangedAttack.generated.h"

class ASMMonsterProjectile;

UCLASS()
class SAGOMAGIC_API UGA_MonsterRangedAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MonsterRangedAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	/** 사용할 공격 애니메이션 */
	UPROPERTY(EditAnywhere, Category = "Design")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** AnimNotify에서 보낼 타격 판정 태그 */
	UPROPERTY(EditAnywhere, Category = "Design")
	FGameplayTag HitEventTag;

	/** 타겟에게 적용할 데미지 GE */
	UPROPERTY(EditAnywhere, Category = "Design")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 투사체 클래스 */
	UPROPERTY(EditAnywhere, Category = "Design")
	TSubclassOf<ASMMonsterProjectile> ProjectileClass;

	/** 투사체 최대 사거리 */
	UPROPERTY(EditAnywhere, Category = "Design")
	float ProjectileRange = 1500.f;

	/** 공격 중 상태 태그 */
	UPROPERTY(EditAnywhere, Category = "Design")
	FGameplayTagContainer AttackingTags;

	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	// 몬스터 AttributeSet에서 공격력 가져오기
	float GetMonsterAttackPower() const;
};
