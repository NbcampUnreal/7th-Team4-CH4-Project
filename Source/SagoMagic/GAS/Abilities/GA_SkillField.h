#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "SkillActor/SMASkillField.h"
#include "GA_SkillField.generated.h"


UCLASS()
class SAGOMAGIC_API UGA_SkillField : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_SkillField();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) override;

	/** 스폰할 장판 액터 클래스 - BP에서 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<ASMASkillField> FieldClass;

	/** 장판 지속 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float FieldDuration = 5.f;

private:
	// 서버에서 클라이언트 타겟데이터 수신 시 호출
	void OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag);

	// 장판 스폰 및 초기화
	void SpawnFieldAtLocation(const FVector& SpawnLocation, const FGameplayAbilityActorInfo* ActorInfo);

	// 마우스 위치
	bool TryGetMouseGroundLocation(APawn* Pawn, FVector& OutLocation) const;
};
