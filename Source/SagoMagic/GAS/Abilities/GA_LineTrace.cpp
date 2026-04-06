// GA_LineTrace.cpp


#include "GA_LineTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GAS/SMAbilitySystemComponent.h"

UGA_LineTrace::UGA_LineTrace()
{
}

void UGA_LineTrace::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	if (IsValid(Avatar) == false || Avatar->HasAuthority() == false)
	{
		//클라이언트 -> 코스메틱 처리 후 종료
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActorInfo(),
			GetCurrentActivationInfo(),
			true,
			false
		);

		return;
	}

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	//DT_Skill로 부터 SkillDuration, DamageInterval 로드
	//if (const FSMSkillData* Row = SkillStatRow.GetRow<FSMSkillData>(TEXT("GA_LineTrace")))
	//{
		////TODO: DT_Skill에 두 Row 추가 후 연결
		////근데 다른 스킬들은 이 항목을 사용하지 않는데 이렇게 Row를 추가하는 형식이 맞느지 의문
		//추후 팀원들과 논의를 통해 구조를 계선하던지 할 필요가 있음
		// SkillDuration = Row->SkillDuration;
		// DamageInterval = Row->DamageInterval;		
	//}

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

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AActor* UGA_LineTrace::FindFirstEnemy(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (IsValid(World) == false) return nullptr;

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
		return HitActor;
	}
	return nullptr;
}

bool UGA_LineTrace::HasAnyTeamTag(AActor* Actor) const
{
	if (IsValid(Actor) == false) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (IsValid(ASC) == false) return false;

	return ASC->HasMatchingGameplayTag(SMSkillTag::Team);
}

void UGA_LineTrace::ApplyDamageTick()
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo) return;

	//매 틱마다 현재 커서 방향으로 AimData 갱신
	ExtractAimData(ActorInfo);

	AActor* HitEnemy = FindFirstEnemy(GetWorld(), ActorInfo);
	if (IsValid(HitEnemy) == false) return; //적 없으면 데미지 없이 틱 스킵 (빔은 유지)

	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	AController* Ctrl = Avatar ? Avatar->GetController() : nullptr;

	//TODO: ExecCal 기반 GE 클래스로 DamageEffectClass로 교체 예정
	//현재는 BP_GE_InstantDamage (단순 SetByCaller) 사용
	USMAbilitySystemComponent::ApplySkillDamage(
		HitEnemy, BaseDamage, Avatar, Ctrl, DamageEffectClass);
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
