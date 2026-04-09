// GA_ApplyInstantDamage.cpp

#include "GA_ApplyInstantDamage.h"

#include "EngineUtils.h"

UGA_ApplyInstantDamage::UGA_ApplyInstantDamage()
{
}

void UGA_ApplyInstantDamage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || ActorInfo->AvatarActor.IsValid() == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. DT에서 스킬 수치 로드
	if (LoadSkillStats() == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//2. 마우스 커서 월드 위치 획득
	if (GetCursorHitLocation(ActorInfo, CursorWorldLocation) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//3. 커서 위치 기준 범위 내 갖장 가까운 적 탐색
	AActor* FoundEnemy = nullptr;
	if (FindClosestEnemy(GetWorld(), CursorWorldLocation, DetectionRadius,
	                     ActorInfo->AvatarActor.Get(), FoundEnemy) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	SelectedTarget = FoundEnemy;

	//4. 적이 있을 때만 커밋 (쿨다운 적용)
	if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//5. 데미지 적용 + 이펙트
	OnSkillEffect(ActorInfo);
}

void UGA_ApplyInstantDamage::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (IsValid(Avatar) == false || Avatar->HasAuthority() == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), false, false)
	}
}

bool UGA_ApplyInstantDamage::GetCursorHitLocation(const FGameplayAbilityActorInfo* ActorInfo,
                                                  FVector& OutLocation) const
{
}

bool UGA_ApplyInstantDamage::FindClosestEnemy(UWorld* World, const FVector& Center, float Radius,
                                              const AActor* IgnoreActor, AActor*& OutEnemy) const
{
}

bool UGA_ApplyInstantDamage::HasAnyTeamTag(AActor* Actor) const
{
}
