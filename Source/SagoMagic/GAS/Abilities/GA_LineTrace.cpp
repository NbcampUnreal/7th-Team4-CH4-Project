// GA_LineTrace.cpp


#include "GA_LineTrace.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GAS/SMAbilitySystemComponent.h"

UGA_LineTrace::UGA_LineTrace()
{
}

void UGA_LineTrace::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	//UE_LOG(LogTemp, Warning, TEXT("[LineTrace] Skill effect is active"));
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	if (IsValid(Avatar) == false || Avatar->HasAuthority() == false)
	{
		// 클라이언트는 코스메틱만 처리하고, 종료를 서버로 전파하지 않는다.
		// TODO: 이펙트 연결
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

	//UE_LOG(LogTemp, Warning, TEXT("[LineTrace] AddGameplayCue called -  RangeCm: %.1f"),RangeCm);
	GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
		SMSkillTag::GameplayCue_Skill_LineTrace_Beam, CueParameters);

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

void UGA_LineTrace::ApplyDamageToActor(AActor* HitActor)
{
	if (!HitActor)
	{
		return;
	}

	// Hit Actor의 ASC 가져오기
	UAbilitySystemComponent* TargetASC = HitActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		return;
	}

	// Damage Effect 적용
	//UE_LOG(LogTemp, Warning, TEXT("[LineTrace] Apply damage to %s, %s"),*HitActor->GetName(), *TargetASC->GetName());
	FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		DamageEffectClass,
		1.0f, // Level
		EffectContext
	);

	if (SpecHandle.IsValid())
	{
		// SetByCaller로 데미지 수치 전달
		SpecHandle.Data->SetSetByCallerMagnitude(SMSkillTag::Data_Damage_Amount, BaseDamage);

		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
			*SpecHandle.Data.Get(),
			TargetASC
		);
	}
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

	return ASC->HasMatchingGameplayTag(SMSkillTag::Team);
}

void UGA_LineTrace::ApplyDamageTick()
{
	//UE_LOG(LogTemp, Warning, TEXT("[LineTrace] Apply tick damage"));
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo) return;

	//매 틱마다 현재 커서 방향으로 AimData 갱신
	ExtractAimData(ActorInfo);

	FHitResult OutHit;
	const bool bHit = FindFirstEnemy(GetWorld(), ActorInfo, OutHit);

	if (bShowDebugTrace)
	{
		const FVector Start = CurrentAimOrigin;
		const FVector End = Start + CurrentAimDirection.GetSafeNormal() * RangeCm;

		// 빔 라인: 맞으면 빨간색, 빗나가면 초록색
		const FColor LineColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(GetWorld(), Start, End, LineColor, false, DamageInterval, 0, 2.0f);

		if (bHit)
		{
			// 히트 지점에 구체 표시
			DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 15.f, 8, FColor::Orange,
			                false, DamageInterval);
		}
	}

	if (bHit)
	{
		// UE_LOG(LogTemp, Warning,
		//        TEXT("[LineTrace] HitActor: %s | ImpactPoint: %s | Distance: %.1f | BoneName: %s"),
		//        *OutHit.GetActor()->GetName(), // 맞은 액터 이름
		//        *OutHit.ImpactPoint.ToString(), // 월드 히트 좌표 (X, Y, Z)
		//        OutHit.Distance, // 시작점에서 히트까지 거리 (cm)
		//        *OutHit.BoneName.ToString() // 맞은 본 이름 (스켈레탈 메시일 경우)
		// );

		if (AActor* HitActor = OutHit.GetActor())
		{
			ApplyDamageToActor(HitActor);
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
