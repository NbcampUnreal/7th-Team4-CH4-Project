// GA_ApplyInstantDamage.cpp

#include "GA_ApplyInstantDamage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
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
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), false, false);
		return;
	}

	AActor* Target = SelectedTarget.Get();
	if (IsValid(Target) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	//데미지 적용
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (TargetASC)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
		if (SpecHandle.IsValid() == true)
		{
			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(
				*SpecHandle.Data.Get(), TargetASC);
		}
	}

	//GameplayCue실행 (낙뢰)
	FGameplayCueParameters CueParams;
	CueParams.Location = Target->GetActorLocation();
	CueParams.EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();

	GetAbilitySystemComponentFromActorInfo()->ExecuteGameplayCue(
		SMSkillTag::GameplayCue_Skill_ApplyInstantDamage_Hit, CueParams);

	EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
}

bool UGA_ApplyInstantDamage::GetCursorHitLocation(const FGameplayAbilityActorInfo* ActorInfo,
                                                  FVector& OutLocation) const
{
	if (!ActorInfo || ActorInfo->AvatarActor.IsValid() == false) return false;

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
	if (bShowDebugSphere == true)
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

	if (OutEnemy && bShowDebugSphere == true)
	{
		DrawDebugLine(World, Center, OutEnemy->GetActorLocation(), FColor::Red,
		              false, 2.0f, 0, 2.0f);
	}

	return OutEnemy != nullptr;
}

bool UGA_ApplyInstantDamage::HasAnyTeamTag(AActor* Actor) const
{
	if (IsValid(Actor) == false) return false;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
	if (IsValid(ASC) == false) return false;

	return ASC->HasMatchingGameplayTag(SMGameFlowTag::Team);
}
