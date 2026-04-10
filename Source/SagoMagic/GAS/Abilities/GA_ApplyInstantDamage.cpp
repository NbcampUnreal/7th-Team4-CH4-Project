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
	if (LoadActiveSkillSummary(ActorInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//2.클라이언트: 마우스 커서 월드 위치 획득 -> 로컬 적 탐색 -> TargetData전송
	APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (IsValid(Pawn) == true && IsValid(ASC) == true && Pawn->HasAuthority() == false)
	{
		FVector CursorWorldLocation = FVector::ZeroVector;
		if (GetCursorHitLocation(ActorInfo, CursorWorldLocation) == false)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		//로컬 적 탐색 (스킬 커밋여부 판단용, 실제 데미지는 서버가 결정)
		AActor* LocalTarget = nullptr;
		if (FindClosestEnemy(GetWorld(), CursorWorldLocation, DetectionRadius,
		                     ActorInfo->AvatarActor.Get(), LocalTarget) == false)
		{
			ASC->ServerSetReplicatedTargetData(
				Handle, ActivationInfo.GetActivationPredictionKey(),
				FGameplayAbilityTargetDataHandle(),
				FGameplayTag(), ASC->ScopedPredictionKey
			);

			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		//4. 적 발견 -> 클라이언트 CommitAbility (예츸 쿨다운)
		if (CommitAbility(Handle, ActorInfo, ActivationInfo) == false)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
		
		// 클라이언트 예측 GameplayCue 실행
		FGameplayCueParameters CueParams;
		CueParams.Location = LocalTarget->GetActorLocation();
		CueParams.EffectContext = ASC->MakeEffectContext();
		ASC->ExecuteGameplayCue(
			SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParams);

		//적이 있으므로 커서 위치를 TargetData로 서버에 전송
		FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
		LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationData->TargetLocation.LiteralTransform = FTransform(CursorWorldLocation);

		FGameplayAbilityTargetDataHandle TargetDataHandle;
		TargetDataHandle.Add(LocationData);

		//서버로 전송
		ASC->ServerSetReplicatedTargetData(
			Handle,
			ActivationInfo.GetActivationPredictionKey(),
			TargetDataHandle,
			FGameplayTag(),
			ASC->ScopedPredictionKey
		);
		
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return; 
	}

	//3. 서버 : CommitAbility 하지 않음 -> OnTargetDataReady에서 적 검증 후 처리
	OnSkillEffect(ActorInfo);
}

void UGA_ApplyInstantDamage::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (IsValid(Avatar) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	if (Avatar->HasAuthority() == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), false, false);
		return;
	}

	//서버: TargetData 수신 대기
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (IsValid(SourceASC) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	SourceASC->AbilityTargetDataSetDelegate(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	).AddUObject(this, &UGA_ApplyInstantDamage::OnTargetDataReady);

	SourceASC->CallReplicatedTargetDataDelegatesIfSet(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	);
}

bool UGA_ApplyInstantDamage::GetCursorHitLocation(const FGameplayAbilityActorInfo* ActorInfo,
                                                  FVector& OutLocation) const
{
	if (!ActorInfo || ActorInfo->PlayerController.IsValid() == false) return false;

	APlayerController* PC = ActorInfo->PlayerController.Get();

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult) == true)
	{
		OutLocation = HitResult.Location;
		return true;
	}

	return false;
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
