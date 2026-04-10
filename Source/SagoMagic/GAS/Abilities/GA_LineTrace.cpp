// GA_LineTrace.cpp


#include "GA_LineTrace.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "GAS/SMAbilitySystemComponent.h"

UGA_LineTrace::UGA_LineTrace()
{
}

void UGA_LineTrace::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	if (IsValid(Avatar) == false)
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			false,
			false
		);
		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	FGameplayCueParameters CueParameters;
	CueParameters.RawMagnitude = RangeCm;
	CueParameters.EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();

	GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
		SMSkillTag::GameplayCue_Skill_LineTrace_Beam, CueParameters);

	if (Avatar->HasAuthority() == false)
	{
		//클라이언트인 경우 타이머만 적용
		World->GetTimerManager().SetTimer(
			DurationEndHandle,
			this,
			&UGA_LineTrace::OnDurationExpired,
			SkillDuration,
			false
		);
	}
	//반복 데미지 타이머 - 매 Tick마다 LineTrace발사
	World->GetTimerManager().SetTimer(
		DamageTickHandle,
		this,
		&UGA_LineTrace::ApplyDamageTick,
		DamageInterval,
		true
	);

	//지속시간 종료 타이머 - DamageDuration후 OnDurationExpired호출
	World->GetTimerManager().SetTimer(
		DurationEndHandle,
		this,
		&UGA_LineTrace::OnDurationExpired,
		SkillDuration,
		false
	);
}

void UGA_LineTrace::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                               bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageTickHandle);
		World->GetTimerManager().ClearTimer(DurationEndHandle);
	}

	if (ActorInfo)
	{
		APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
		if (IsValid(Avatar) == true && Avatar->HasAuthority() == true)
		{
			GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(
				SMSkillTag::GameplayCue_Skill_LineTrace_Beam
			);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_LineTrace::FindFirstEnemy(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo, FHitResult& OutHit) const
{
	if (IsValid(World) == false) return false;

	const FVector Start = CurrentAimOrigin;
	const FVector End = Start + CurrentAimDirection.GetSafeNormal() * RangeCm;

	FCollisionQueryParams CollisionParams;
	if (ActorInfo && ActorInfo->AvatarActor.IsValid() == true)
	{
		CollisionParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());
	}

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByChannel(HitResults, Start, End, ECC_Pawn, CollisionParams);

	for (const auto& HitResult : HitResults)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) == false) continue;
		if (HasAnyTeamTag(HitActor) == true) continue;
		OutHit = HitResult;
		return true;
	}
	return false;
}

bool UGA_LineTrace::HasAnyTeamTag(AActor* Actor) const
{
	if (IsValid(Actor) == false) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (IsValid(ASC) == false) return false;

	return ASC->HasMatchingGameplayTag(SMGameFlowTag::Team);
}

void UGA_LineTrace::ApplyDamageTick()
{
	if (IsActive() == false) return;

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo) return;

	//매 틱마다 현재 커서 방향으로 AimData 갱신
	ExtractAimData(ActorInfo);

	FHitResult OutHit;
	const bool bHit = FindFirstEnemy(GetWorld(), ActorInfo, OutHit);

	if (bShowDebugTrace == true && GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		const FVector Start = CurrentAimOrigin;
		const FVector End = Start + CurrentAimDirection.GetSafeNormal() * RangeCm;

		// 빔 라인: 맞으면 빨간색, 빗나가면 초록색
		const FColor LineColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), Start, End, LineColor, false, DamageInterval, 0, 2.0f);

		if (bHit == true)
		{
			// 히트 지점에 구체 표시
			DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 15.f, 8, FColor::Orange,
			                false, DamageInterval);
		}
	}

	if (bHit == true)
	{
		if (AActor* HitActor = OutHit.GetActor())
		{
			UAbilitySystemComponent* TargetASC = HitActor->FindComponentByClass<UAbilitySystemComponent>();
			if (TargetASC)
			{
				FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
				if (SpecHandle.IsValid() == true)
				{
					GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
						*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
	}
}


void UGA_LineTrace::OnDurationExpired()
{
	if (IsActive() == false) return;

	EndAbility(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActorInfo(),
		GetCurrentActivationInfo(),
		true,
		false
	);
}
