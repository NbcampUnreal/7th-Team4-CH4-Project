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
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;

	virtual void OnMontageFinished() override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
	                        const FGameplayAbilityActorInfo* ActorInfo,
	                        const FGameplayAbilityActivationInfo ActivationInfo,
	                        bool bReplicateEndAbility,
	                        bool bWasCancelled) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack|Debug")
	bool bShowDebugTrace = true;

private:
	// LineTrace 결과를 OutHit으로 반환. 적 발견 시 true, 아니면 false.
	// (Team 태그 보유 액터는 관통하며 계속 탐색)
	bool FindFirstEnemy(UWorld* World,
	                    const FGameplayAbilityActorInfo* ActorInfo,
	                    FHitResult& OutHit) const;

	//ASC에서 Team태그 확인
	bool HasAnyTeamTag(AActor* Actor) const;

	// 반복 데미지 틱 콜백: 매 틱마다 LineTrace → 맞으면 데미지, 아니면 스킵
	void ApplyDamageTick();

	//기본 공격 로직, 빔에 맞는 첫번제 적에게만 데미지
	void NormalAttack(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo);	
	
	// 지속시간 종료 콜백: EndAbility 호출
	void OnDurationExpired();

	FTimerHandle DamageTickHandle;
	FTimerHandle DurationEndHandle;
	
	//관통 공격, 빔에 맞는 모든 적에게 데미지
	void PenetrateAttack(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo);
	
	bool FindAllEnemies(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo,
					TArray<AActor*>& OutEnemies) const;
	
	bool bIsPenetrate = false;
	
	//체인 공격
	void ChainAttack();
	
	bool FindNearestEnemy(UWorld* World, const FVector& Origin,
					  float SearchRadius,
					  const TArray<AActor*>& ExcludeActors,
					  AActor*& OutEnemy) const;
	
	UPROPERTY(EditDefaultsOnly, Category="Attack|Chain")
	int32 MaxChainCount = 4;
	
	UPROPERTY(EditDefaultsOnly, Category="Attack|Chain")
	float ChainSearchRadius = 800.f;	
	
	bool bIsChainAttacking = false;
};
