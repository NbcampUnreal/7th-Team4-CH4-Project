// GA_LineTrace.cpp


#include "GA_LineTrace.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/SMPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_LineTrace::UGA_LineTrace()
{
}

void UGA_LineTrace::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
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

	bIsPenetrate = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_LineTrace_Penetrate);
	bIsChainAttacking = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_LineTrace_Chain);
	ChainSearchRadius = RangeCm * ChainSearchRadiusMultiplier;

	FGameplayCueParameters CueParameters;
	CueParameters.RawMagnitude = RangeCm;
	CueParameters.EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	if (bIsChainAttacking == true)
	{
		CueParameters.NormalizedMagnitude = (float)MaxChainCount;
	}
	else
	{
		CueParameters.NormalizedMagnitude = bIsPenetrate ? 1.0f : 0.0f;
	}

	// CashedSummary에서 Duration / TickInterval 읽기
	const float SkillDuration = FieldDuration > 0.0f ? FieldDuration : 3.0f;

	const float DamageInterval = TickInterval > 0.0f ? TickInterval : 0.1f;

	if (Avatar->HasAuthority() == false)
	{
		// 클라: 큐 예측 + 종료 타이머
		if (bIsChainAttacking == true)
		{
			GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
				SMSkillTag::GameplayCue_Skill_LineTrace_Chain, CueParameters);
		}
		else
		{
			GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
				SMSkillTag::GameplayCue_Skill_LineTrace_Beam, CueParameters);
		}

		//StaffTip위치 전송을 위한 Timer실행
		World->GetTimerManager().SetTimer(
			DamageTickHandle,
			this,
			&UGA_LineTrace::ApplyDamageTick,
			DamageInterval,
			true
		);

		World->GetTimerManager().SetTimer(
			DurationEndHandle,
			this,
			&UGA_LineTrace::OnDurationExpired,
			SkillDuration,
			false
		);
		return;
	}

	// 서버: 큐 권한 추가 + 두 타이머
	// 반복 데미지 타이머 - 매 Tick마다 LineTrace발사
	if (bIsChainAttacking == true)
	{
		GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
			SMSkillTag::GameplayCue_Skill_LineTrace_Chain, CueParameters);
	}
	else
	{
		GetAbilitySystemComponentFromActorInfo()->AddGameplayCue(
			SMSkillTag::GameplayCue_Skill_LineTrace_Beam, CueParameters);
	}

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

void UGA_LineTrace::OnMontageFinished()
{
	// 시작 애니메이션 종료 시 부모의 EndAbility 호출 X
	// 빔 종료는 DurationEndHandle -> OnDurationExpired가 EndAbility 호출
}

void UGA_LineTrace::EndAbility(const FGameplayAbilitySpecHandle Handle,
                               const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo,
                               bool bReplicateEndAbility,
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
			if (bIsChainAttacking == true)
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(
					SMSkillTag::GameplayCue_Skill_LineTrace_Chain
				);
			}
			else
			{
				GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(
					SMSkillTag::GameplayCue_Skill_LineTrace_Beam
				);
			}
		}

		if (ActorInfo->AbilitySystemComponent.IsValid())
		{
			// 현재 재생중인 몽타주 강제 종료
			ActorInfo->AbilitySystemComponent->CurrentMontageStop();
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

	while (true)
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, CollisionParams) == false) break;

		AActor* HitActor = Hit.GetActor();
		if (IsValid(HitActor) == false) break;

		CollisionParams.AddIgnoredActor(HitActor);

		if (HasAnyTeamTag(HitActor) == true) continue;
		if (IsAvailableEnemy(HitActor) == false) continue;

		OutHit = Hit;
		return true;
	}
	return false;
}

void UGA_LineTrace::ApplyDamageTick()
{
	if (IsActive() == false) return;

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo) return;

	UWorld* World = GetWorld();
	if (IsValid(World) == false) return;

	// ApplyDamageTick 호출 시마다 현재 캐릭터 위치 및 컨트롤러 방향으로 AimData 갱신
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (IsValid(Avatar) == false) return;

	ASMPlayerCharacter* Character = Cast<ASMPlayerCharacter>(Avatar);
	if (IsValid(Character) == false) return;

	if (Avatar->HasAuthority() == false)
	{
		Character->ServerSetStaffTipAimOrigin(GetStaffTipLocation(Avatar));
		return;
	}

	CurrentAimOrigin = Character->ServerStaffTipLocation.IsZero()
		                   ? Avatar->GetActorLocation()
		                   : Character->ServerStaffTipLocation;

	if (AController* Controller = Avatar->GetController())
	{
		CurrentAimDirection = Controller->GetControlRotation().Vector();
	}

	if (bIsPenetrate == true)
	{
		PenetrateAttack(World, ActorInfo);
	}
	else if (bIsChainAttacking == true)
	{
		ChainAttack(World, ActorInfo);
	}
	else
	{
		NormalAttack(World, ActorInfo);
	}
}

void UGA_LineTrace::NormalAttack(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (IsValid(World) == false) return;

	FHitResult OutHit;
	const bool bHit = FindFirstEnemy(World, ActorInfo, OutHit);

	if (bShowDebugTrace == true && World->GetNetMode() != NM_DedicatedServer)
	{
		const FVector Start = CurrentAimOrigin;
		const FVector End = Start + CurrentAimDirection.GetSafeNormal() * RangeCm;

		const float FinalTickInterval =
			CachedSummary.GetFinalTickInterval() ? CachedSummary.GetFinalTickInterval() : 0.1f;

		// 빔 라인: 맞으면 빨간색, 빗나가면 초록색
		const FColor LineColor = bHit ? FColor::Red : FColor::Green;
		DrawDebugLine(
			GetWorld(),
			Start,
			End,
			LineColor,
			false,
			FinalTickInterval,
			0,
			2.0f);

		if (bHit == true)
		{
			// 히트 지점에 구체 표시
			DrawDebugSphere(
				GetWorld(),
				OutHit.ImpactPoint,
				15.f,
				8,
				FColor::Orange,
				false,
				FinalTickInterval);
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
						*SpecHandle.Data.Get(),
						TargetASC);
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

void UGA_LineTrace::PenetrateAttack(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo)
{
	TArray<AActor*> OutEnemies;

	if (FindAllEnemies(World, ActorInfo, OutEnemies) == false) return;

	for (AActor* Enemy : OutEnemies)
	{
		if (UAbilitySystemComponent* TargetASC =
			Enemy->FindComponentByClass<UAbilitySystemComponent>())
		{
			FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
			if (SpecHandle.IsValid())
			{
				GetAbilitySystemComponentFromActorInfo()
					->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

bool UGA_LineTrace::FindAllEnemies(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo,
                                   TArray<AActor*>& OutEnemies) const
{
	if (IsValid(World) == false) return false;

	const FVector Start = CurrentAimOrigin;
	const FVector End = Start + CurrentAimDirection.GetSafeNormal() * RangeCm;

	FCollisionQueryParams CollisionParams;
	if (ActorInfo && ActorInfo->AvatarActor.IsValid() == true)
	{
		CollisionParams.AddIgnoredActor(ActorInfo->AvatarActor.Get());
	}

	//collision 채널 문제로 singleTrace를 여러번 쏴서 판단하여 추가 OutEnemies에 추가
	while (true)
	{
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, CollisionParams) == false)
			break; // 더 이상 hit 없음

		AActor* HitActor = Hit.GetActor();
		if (IsValid(HitActor) == false) break;

		CollisionParams.AddIgnoredActor(HitActor); // 다음 트레이스에서 무시

		if (HasAnyTeamTag(HitActor) == true) continue; // 아군은 스킵
		if (IsAvailableEnemy(HitActor) == false) continue; //적군 태그 없으면 스킵

		OutEnemies.Add(HitActor);
	}

	return OutEnemies.Num() > 0;
}


void UGA_LineTrace::ChainAttack(UWorld* World, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (IsValid(World) == false) return;

	// 1. 첫 번째 적 탐색 (기존 FindFirstEnemy 재사용)
	FHitResult FirstHit;
	if (FindFirstEnemy(World, ActorInfo, FirstHit) == false) return;

	AActor* FirstEnemy = FirstHit.GetActor();
	if (IsValid(FirstEnemy) == false) return;

	// 2. 체인 배열 초기화
	TArray<AActor*> ChainedEnemies;
	ChainedEnemies.Add(FirstEnemy);

	// 3. 첫 번째 적에게 데미지
	if (UAbilitySystemComponent* TargetASC =
		FirstEnemy->FindComponentByClass<UAbilitySystemComponent>())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
		if (SpecHandle.IsValid())
		{
			GetAbilitySystemComponentFromActorInfo()
				->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}

	// 4. 최대 MaxChainCount - 1번 추가 체인
	//    (첫 번째 적이 이미 1번이므로 MaxChainCount - 1번 더 탐색)
	for (int32 ChainIndex = 1; ChainIndex < MaxChainCount; ++ChainIndex)
	{
		// 마지막으로 체인된 적의 위치에서 가장 가까운 새 적 탐색
		const FVector SearchOrigin = ChainedEnemies.Last()->GetActorLocation();
		AActor* NextEnemy = nullptr;
		if (FindNearestEnemy(World, SearchOrigin, ChainSearchRadius,
		                     ChainedEnemies, NextEnemy) == false)
			break; // 더 이상 튕길 대상 없음

		ChainedEnemies.Add(NextEnemy);

		// 체인 대상에게 데미지
		if (UAbilitySystemComponent* TargetASC =
			NextEnemy->FindComponentByClass<UAbilitySystemComponent>())
		{
			FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
			if (SpecHandle.IsValid())
			{
				GetAbilitySystemComponentFromActorInfo()
					->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

bool UGA_LineTrace::FindNearestEnemy(UWorld* World, const FVector& Origin, float SearchRadius,
                                     const TArray<AActor*>& ExcludeActors, AActor*& OutEnemy) const
{
	if (IsValid(World) == false) return false;

	// ObjectTypes: SMASkillProjectile과 동일한 타입 사용
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Origin,
		SearchRadius,
		ObjectTypes,
		nullptr, // 필터 클래스 없음
		ExcludeActors, // 이미 체인된 적 + 시전자 자동 제외
		OverlapActors
	);

	float NearestDistSq = FLT_MAX;

	for (AActor* Actor : OverlapActors)
	{
		if (IsValid(Actor) == false) continue;
		if (HasAnyTeamTag(Actor) == true) continue; // 아군 제외
		if (IsAvailableEnemy(Actor) == false) continue; //적군 태그 없으면 제외

		const float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			OutEnemy = Actor;
		}
	}
	return IsValid(OutEnemy); // false면 체인 종료
}

FVector UGA_LineTrace::GetStaffTipLocation(APawn* Avatar) const
{
	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (IsValid(Character) == false) return Avatar->GetActorLocation();

	TArray<UStaticMeshComponent*> Comps;
	Character->GetComponents<UStaticMeshComponent>(Comps);
	for (UStaticMeshComponent* Comp : Comps)
	{
		if (IsValid(Comp) == false) continue;
		if (Comp->GetAttachSocketName() != FName("WeaponSocket")) continue;
		if (Comp->DoesSocketExist(FName("Staff_Tip")))
		{
			return Comp->GetSocketLocation(FName("Staff_Tip"));
		}
		break;
	}
	return Avatar->GetActorLocation();
}
