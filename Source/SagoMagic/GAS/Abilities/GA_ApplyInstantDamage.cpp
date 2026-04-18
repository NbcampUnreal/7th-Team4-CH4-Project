// GA_ApplyInstantDamage.cpp

#include "GA_ApplyInstantDamage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_ApplyInstantDamage::UGA_ApplyInstantDamage()
{
}

void UGA_ApplyInstantDamage::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!Avatar) return;

	DetectionRadius = RangeCm;
	bIsInstantMulti = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_ApplyInstantDamage_InstantMulti);
	bIsSeparateMulti = SkillUpgradeTags.HasTag(SMSkillTag::Upgrade_ApplyInstantDamage_SeparateMulti);

	if (bIsSeparateMulti) TargetCount = SeparateMultiTargetCount;
	else if (bIsInstantMulti) TargetCount = InstantMultiTargetCount;
	else TargetCount = 1;

	// ── 클라이언트 예측: 반경 내 타겟에게 Cue 즉시 발동 ──
	if (Avatar->IsLocallyControlled() && Avatar->HasAuthority() == false)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (IsValid(ASC) == false) return;

		if (bIsSeparateMulti)
		{
			if (CastSound)
			{
				UGameplayStatics::PlaySoundAtLocation(Avatar, CastSound, Avatar->GetActorLocation());
			}
			return;
		}

		TArray<AActor*> Enemies;
		bool bFound = FindClosestEnemies(GetWorld(), TargetLocation, DetectionRadius, Avatar, TargetCount, Enemies);

		if (bFound == false) return;

		// 시전 사운드 - 로컬에서 항상 재생
		if (CastSound)
		{
			UGameplayStatics::PlaySoundAtLocation(Avatar, CastSound, Avatar->GetActorLocation());
		}

		for (AActor* Enemy : Enemies)
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = Enemy->GetActorLocation();
			CueParams.EffectContext = ASC->MakeEffectContext();
			ASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParams);
		}
		return;
	}

	// 서버에서는 실제 데미지 적용 및 이펙트 복제
	if (Avatar->HasAuthority() == false) return;
	
	if (bIsSeparateMulti)
	{
		SeparateMultiAttack(ActorInfo, TargetLocation);
	}
	else if (bIsInstantMulti)
	{
		InstantMultiAttack(ActorInfo, TargetLocation);
	}
	else
	{
		NormalAttack(ActorInfo, TargetLocation);
	}
}

void UGA_ApplyInstantDamage::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                        const FGameplayAbilityActorInfo* ActorInfo,
                                        const FGameplayAbilityActivationInfo ActivationInfo,
                                        bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LightningTimerHandle);
	}

	EnemyCandidates.Reset();
	bLightningPending = false;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_ApplyInstantDamage::OnMontageFinished()
{
	if (bLightningPending == true) return;
	Super::OnMontageFinished();
}


void UGA_ApplyInstantDamage::NormalAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	TArray<AActor*> Enemies;
	if (FindClosestEnemies(GetWorld(), Center, DetectionRadius, Avatar, TargetCount, Enemies) == false) return;

	ApplyDamageAndCue(ActorInfo, Avatar, Enemies[0]);
}

void UGA_ApplyInstantDamage::ApplyDamageAndCue(const FGameplayAbilityActorInfo* ActorInfo, APawn* Avatar,
                                               AActor* Target)
{
	if (!ActorInfo || IsValid(Target) == false) return;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(SourceASC) == false) return;

	UAbilitySystemComponent* TargetASC =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (IsValid(TargetASC))
	{
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
		if (SpecHandle.IsValid())
		{
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}

	FGameplayCueParameters CueParams;
	CueParams.Location = Target->GetActorLocation();
	CueParams.EffectContext = SourceASC->MakeEffectContext();
	SourceASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParams);
}

bool UGA_ApplyInstantDamage::FindClosestEnemies(UWorld* World, const FVector& Center, float Radius,
                                                const AActor* IgnoreActor, int32 MaxCount,
                                                TArray<AActor*>& OutEnemies) const
{
	OutEnemies.Reset();

	if (IsValid(World) == false) return false;

	TArray<AActor*> OverlapActors;
	TArray<AActor*> ActorsToIgnore;
	if (IgnoreActor)
	{
		ActorsToIgnore.Add(const_cast<AActor*>(IgnoreActor));
	}

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	UKismetSystemLibrary::SphereOverlapActors(
		World, Center, Radius, ObjectTypes, nullptr, ActorsToIgnore, OverlapActors);

	if (bShowDebugSphere == true && World->GetNetMode() != NM_DedicatedServer)
	{
		DrawDebugSphere(World, Center, Radius, 16, FColor::Cyan,
		                false, 2.0f, 0, 1.0f);
	}

	//유효한 적만 (거리, Actor) 쌍으로 수집
	TArray<TPair<float, AActor*>> Candidates;
	for (AActor* Actor : OverlapActors)
	{
		if (IsValid(Actor) == false) continue;
		if (HasAnyTeamTag(Actor) == true) continue;
		if (IsAvailableEnemy(Actor) == false) continue;

		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (IsValid(ASC) == false) continue;

		const float DistSq = FVector::DistSquared(Center, Actor->GetActorLocation());
		Candidates.Add(TPair<float, AActor*>(DistSq, Actor));
	}
	//거리 오름차순 정렬
	Candidates.Sort(
		[](const TPair<float, AActor*>& a, const TPair<float, AActor*>& b)
		{
			return a.Key < b.Key;
		});

	//상위 MaxCount명 추출
	const int32 Count = FMath::Min(MaxCount, Candidates.Num());
	for (int32 i = 0; i < Count; i++)
	{
		OutEnemies.Add(Candidates[i].Value);

		if (bShowDebugSphere == true && World->GetNetMode() != NM_DedicatedServer)
		{
			DrawDebugLine(World, Center, Candidates[i].Value->GetActorLocation(), FColor::Red,
			              false, 2.0f, 0, 2.0f);
		}
	}

	return OutEnemies.Num() > 0;
}

//다중 적 공격
void UGA_ApplyInstantDamage::InstantMultiAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	TArray<AActor*> Enemies;
	if (FindClosestEnemies(GetWorld(), Center, DetectionRadius, Avatar, TargetCount, Enemies) == false) return;

	for (AActor* Enemy : Enemies)
	{
		ApplyDamageAndCue(ActorInfo, Avatar, Enemy);
	}
}

//다중 적 다중 공격

void UGA_ApplyInstantDamage::SeparateMultiAttack(const FGameplayAbilityActorInfo* ActorInfo, const FVector& Center)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());

	TArray<AActor*> Enemies;
	if (FindClosestEnemies(GetWorld(), Center, DetectionRadius, Avatar, TargetCount, Enemies) == false) return;


	EnemyCandidates = Enemies;
	RemainingLightnings = SeparateMultiLightningCount;
	LastHitTarget = nullptr;
	bLightningPending = true;

	//첫 낙뢰 즉시 발동, 이후 LightningDelay 간격 반복
	GetWorld()->GetTimerManager().SetTimer(
		LightningTimerHandle,
		this,
		&UGA_ApplyInstantDamage::FireNextLightningBolt,
		LightningDelay,
		true,
		0.f);
}

void UGA_ApplyInstantDamage::FireNextLightningBolt()
{
	//유효하지 않은 적들 후보에서 제거
	EnemyCandidates.RemoveAll([this](AActor* Actor)
	{
		return IsValid(Actor) == false || IsAvailableEnemy(Actor) == false;
	});

	//살아있는 적이 없으면 남은 발수와 관게 없이 종료
	if (EnemyCandidates.IsEmpty() == true)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
		           GetCurrentActivationInfo(), true, false);
		return;
	}

	// LastHitTarget 제외 후보 수집 (연속 동일 타겟 방지)
	TArray<AActor*> HitCandidates;
	for (AActor* Enemy : EnemyCandidates)
	{
		if (Enemy != LastHitTarget.Get())
		{
			HitCandidates.Add(Enemy);
		}
	}
	//후보 없으면 (남은적이 1명이다) 제한해제
	if (HitCandidates.IsEmpty() == true)
	{
		HitCandidates = EnemyCandidates;
	}

	AActor* Target = HitCandidates[FMath::RandRange(0, HitCandidates.Num() - 1)];
	LastHitTarget = Target;

	APawn* Avatar = Cast<APawn>(GetCurrentActorInfo()->AvatarActor.Get());
	ApplyDamageAndCue(GetCurrentActorInfo(), Avatar, Target);

	// 발수 차감 후 소진 시 종료
	RemainingLightnings--;
	if (RemainingLightnings <= 0)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(),
		           GetCurrentActivationInfo(), true, false);
	}
}
