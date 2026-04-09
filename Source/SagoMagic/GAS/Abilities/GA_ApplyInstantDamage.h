// GA_ApplyInstantDamage.h

#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "GA_ApplyInstantDamage.generated.h"

/**
 * 마우스 커서 위치 기준 원형 범위 내 가장 가까운 적에게 즉시 낙뢰 데미지를 주는 스킬입니다.
 * 범위 내 적이 없으면 어빌리티가 취소되며 쿨다운이 소비되지 않습니다.
 */
UCLASS()
class SAGOMAGIC_API UGA_ApplyInstantDamage : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_ApplyInstantDamage();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) override;

	/** 커서 위치 기준 적 탐색 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DetectionRadius")
	float DetectionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Debug")
	bool bShowDebugSphere = true;

private:
	// PlayerController에서 커서가 가리키는 월드 좌표를 가져옴
	bool GetCursorHitLocation(const FGameplayAbilityActorInfo* ActorInfo, FVector& OutLocation) const;

	//지정 위치 기준 반경 내 가장 가까운 적을 찾음 성공 시 true, OutEnemy에 결과 저장
	bool FindClosestEnemy(UWorld* World, const FVector& Center, float Radius, const AActor* IgnoreActor,
	                      AActor*& OutEnemy) const;
	
	//아군 태그(Team) 보유 여부 확인
	bool HasAnyTeamTag(AActor* Actor) const;
	
	FVector CursorWorldLocation = FVector::ZeroVector;
	
	TWeakObjectPtr<AActor> SelectedTarget;
	
};
