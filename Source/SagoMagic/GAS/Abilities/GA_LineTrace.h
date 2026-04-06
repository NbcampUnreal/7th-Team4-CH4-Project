// GA_LineTrace.h

#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "GA_LineTrace.generated.h"

/**
 * 라인트레이스를 발사하여 첫번째로 닿는 적에게 데미지를 주는 빔 스킬입니다.
 */
UCLASS()
class SAGOMAGIC_API UGA_LineTrace : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_LineTrace();

protected:
	virtual void OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
	                        bool bWasCancelled) override;

private:
	//LineTrace에 맞는 첫 번째 적 반환 (Team태그 보유 액터는 관통)
	AActor* FindFirstEnemy(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo) const;

	//ASC에서 Team태그 확인
	bool HasAnyTeamTag(AActor* Actor) const;

	// 반복 데미지 틱 콜백: 매 틱마다 LineTrace → 맞으면 데미지, 아니면 스킵
	void ApplyDamageTick();

	// 지속시간 종료 콜백: EndAbility 호출
	void OnDurationExpired();

	FTimerHandle DamageTickHandle;
	FTimerHandle DurationEndHandle;

	// 나중에 DT에 연결하여 값 사용
	// 테스트용 하드코딩
	float SkillDuration = 3.0f;
	float DamageInterval = 0.1f;
};
