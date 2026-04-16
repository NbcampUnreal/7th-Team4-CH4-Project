#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataTable.h"
#include "Inventory/Core/SMSkillRuntimeTypes.h"
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

	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags) const override;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void ApplyCooldown(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	/** 현재 Prediction Key를 반환하고 더 많은? 예측에 유효한지 Valid 한다 - 디버깅에 사용 */
	UFUNCTION(BlueprintCallable, Category = "SagoMagic|Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	/** 현재 Prediction Key가 추가 예측에 유효한지 -> Prediction Key가 여전히 사용 가능한지 여부 확인 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SagoMagic|Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

protected:
	virtual void ExecuteSkillLogic(const FGameplayAbilityActorInfo* ActorInfo);

	//스킬효과 서버
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo, const FVector& TargetLocation, const FVector& AimDirection) {}
	
	// 몽타지 끝 콜백 함수
	UFUNCTION()
	virtual void OnMontageFinished();

	//OnSkillEffect 내에서 호출. 유효하지 않으면 Invalid Handle 반환.
	FGameplayEffectSpecHandle MakeDamageSpec(const FGameplayAbilityActorInfo* ActorInfo) const;

	/** 쿨다운중 쿨다운 재발동 차단용 ex) Cooldown.Skill.Projectile */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
	FGameplayTag CooldownTag;

	/** DT_SkillData에서 스킬 행 참조 - 에디터에서 테이블과 행 이름 지정 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Stats")
	FDataTableRowHandle SkillStatRow;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Animation")
	FGameplayTag SkillEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SagoMagic|Skill")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// ActivateAbility시 인벤토리에서 채워지는 스탯
	float BaseDamage = 0.f;  // 데미지
	float RangeCm  = 0.f;  // 사거리/범위
	float CooldownSeconds = 0.f;  // 쿨다운
	float FieldDuration = 0.f;  // 지속 시간
	float TickInterval = 0.f;  // 틱 간격
	FGameplayTagContainer SkillUpgradeTags; // 스킬 업그레이드 태그
	
	TArray<FGuid> EmbeddedGemIds;           // 장착된 젬 ID 목록
	TArray<FGuid> EmbeddedSkillIds;         // 장착된 동일 스킬 ID 목록
	
	//인벤토리에서 받아온 최종 요약 캐시
	FSMCompiledSkillSummary CachedSummary;

	//인벤토리에서 최종 계산값 로드
	bool LoadActiveSkillSummary(const FGameplayAbilityActorInfo* ActorInfo);

	FVector CurrentAimOrigin = FVector::ZeroVector;
	FVector CurrentAimDirection = FVector::ForwardVector;
	FVector CurrentTargetLocation = FVector::ZeroVector;
	
	// 서버에서 데이터 수신하는 함수
	void OnTargetDataReadyCallBack(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	                               FGameplayTag ApplicationTag);

	// 몽타주와 이벤트 대기 Task를 실행하는 함수
	virtual void StartMontageAndTasks();

	// 이벤트 델리게이트 함수
	UFUNCTION()
	virtual void OnFireEventReceived(FGameplayEventData Payload);
	
	// 로컬에서 마우스가 가리키는 바닥 좌표 구하는 함수
	bool TryGetMouseGroundLocation(APawn* Pawn, FVector& OutLocation) const;
};
