// GA_ApplyInstantDamage.cpp

#include "GA_ApplyInstantDamage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "DrawDebugHelpers.h"
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
	
	// 부모가 넘겨준 TargetLocation 기준으로 가까운 적 찾기
	AActor* FoundEnemy = nullptr;
	bool bFound = FindClosestEnemy(GetWorld(), TargetLocation, DetectionRadius, Avatar, FoundEnemy);
	
	// 적중 이펙트 예측
	if (Avatar->IsLocallyControlled() && !Avatar->HasAuthority())
	{
		if (bFound && FoundEnemy)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (ASC)
			{
				FGameplayCueParameters CueParmas;
				CueParmas.Location = FoundEnemy->GetActorLocation();
				CueParmas.EffectContext = ASC->MakeEffectContext();
				ASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParmas);
			}
		}
		return;
	}
	
	// 서버에서는 실제 데미지 적용 및 이펙트 복제
	if (Avatar->HasAuthority())
	{
		if (bFound && FoundEnemy)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(FoundEnemy);
			if (TargetASC)
			{
				// 데미지 적용
				FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
				if (SpecHandle.IsValid())
				{
					GetAbilitySystemComponentFromActorInfo()->
						ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
			
			UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
			if (SourceASC)
			{
				FGameplayCueParameters CueParmas;
				CueParmas.Location = FoundEnemy->GetActorLocation();
				CueParmas.EffectContext = SourceASC->MakeEffectContext();
				SourceASC->ExecuteGameplayCue(SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParmas);
			}
		}
	}
}

bool UGA_ApplyInstantDamage::FindClosestEnemy(UWorld* World, const FVector& Center, float Radius,
                                              const AActor* IgnoreActor, AActor*& OutEnemy) const
{
	OutEnemy = nullptr;

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

	//아군, ASC 없는 액터 제외 가장 가까운 적 선택
	float ClosestDistSq = FLT_MAX;

	for (AActor* Actor : OverlapActors)
	{
		if (IsValid(Actor) == false) continue;
		//아군 확인
		if (HasAnyTeamTag(Actor) == true) continue;

		//ASC없는 액터 확인
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (IsValid(ASC) == false) continue;

		const float DistSq = FVector::DistSquared(Center, Actor->GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			OutEnemy = Actor;
		}
	}

	if (OutEnemy && bShowDebugSphere == true && World->GetNetMode() != NM_DedicatedServer)
	{
		DrawDebugLine(World, Center, OutEnemy->GetActorLocation(), FColor::Red,
		              false, 2.0f, 0, 2.0f);
	}

	return OutEnemy != nullptr;
}

void UGA_ApplyInstantDamage::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
                                               FGameplayTag ApplicationTag)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || ActorInfo->AvatarActor.IsValid() == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	//빈 TargetData = 클라이언트가 적을 못찾음 -> 쿨다운 없이 종료
	if (TargetDataHandle.Num() == 0)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (IsValid(Avatar) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	//1. TargetData에서 클라이언트가 보낸 커서 위치 추출
	FVector CursorLocation = Avatar->GetActorLocation();
	if (const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(0))
	{
		CursorLocation = TargetData->GetEndPoint();
	}

	//2. 서버가 직접 적 탐색 (서버 검증)
	AActor* FoundEnemy = nullptr;
	if (FindClosestEnemy(GetWorld(), CursorLocation, DetectionRadius, Avatar, FoundEnemy) == false)
	{
		// 서버에서 적 못 찾음 -> 쿨다운 없이 종료 (클라이언트 예측 쿨다운 롤백됨)
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	//3. 서버 CommitAbility - 적을 검증한 후에만 실행해서 쿨다운 적용
	if (CommitAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo()) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	//4.데미지 적용
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(FoundEnemy);
	if (TargetASC)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
		if (SpecHandle.IsValid() == true)
		{
			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
				*SpecHandle.Data.Get(), TargetASC);
		}
	}

	//5.GameplayCue(낙뢰) 실행 (서버 ASC를 통해 모든 클라이언트에 복제)
	FGameplayCueParameters CueParams;
	CueParams.Location = FoundEnemy->GetActorLocation();
	CueParams.EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParams);

	EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}

bool UGA_ApplyInstantDamage::HasAnyTeamTag(AActor* Actor) const
{
	if (IsValid(Actor) == false) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (IsValid(ASC) == false) return false;

	return ASC->HasMatchingGameplayTag(SMGameFlowTag::Team);
}
