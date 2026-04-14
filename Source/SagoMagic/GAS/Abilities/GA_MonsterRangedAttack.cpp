#include "GAS/Abilities/GA_MonsterRangedAttack.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Enemy/SMMonsterAIController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterProjectile.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"

UGA_MonsterRangedAttack::UGA_MonsterRangedAttack()
{
	// 서버에서만 실행되게
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MonsterRangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 공격 상태 태그 부착
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTags(AttackingTags);
	}

	// IsAttacking = true → BT에서 MoveTo 중단
	if (APawn* MonsterPawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (ASMMonsterAIController* AIC = Cast<ASMMonsterAIController>(MonsterPawn->GetController()))
		{
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(FName("IsAttacking"), true);
			}
			AIC->StopMovement();
		}
	}

	// 몽타주 재생
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, FName("RangedAttack"), AttackMontage, 0.7f);

	MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterRangedAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterRangedAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterRangedAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// AnimNotify 이벤트 대기
	UAbilityTask_WaitGameplayEvent* WaitEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag);

	WaitEventTask->EventReceived.AddDynamic(this, &UGA_MonsterRangedAttack::OnHitEventReceived);
	WaitEventTask->ReadyForActivation();
}

void UGA_MonsterRangedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 공격 상태 태그 해제
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTags(AttackingTags);
	}

	// IsAttacking 해제는 AIController 타이머가 담당
	// 타겟이 범위 밖으로 나갔을 때만 해제 → 범위 내면 계속 제자리 공격

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MonsterRangedAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	if (!GetActorInfo().IsNetAuthority())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	if (!SourceActor || !SourceASC || !ProjectileClass || !DamageEffectClass)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// CurrentAttackTarget을 향한 방향 계산
	FVector FireDirection = SourceActor->GetActorForwardVector();
	if (APawn* MonsterPawn = Cast<APawn>(SourceActor))
	{
		if (ASMMonsterAIController* AIController = Cast<ASMMonsterAIController>(MonsterPawn->GetController()))
		{
			if (AActor* Target = AIController->CurrentAttackTarget)
			{
				const FVector ToTarget = Target->GetActorLocation() - SourceActor->GetActorLocation();
				if (!ToTarget.IsNearlyZero())
				{
					FireDirection = ToTarget.GetSafeNormal();
				}
			}
		}
	}

	// GE Spec 생성
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.AddInstigator(SourceActor,
	                            Cast<APawn>(SourceActor) ? Cast<APawn>(SourceActor)->GetController() : nullptr);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(),
	                                                                   EffectContext);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag(TEXT("Data.Damage.Amount")), -GetMonsterAttackPower());
	}

	// 투사체 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SourceActor;
	SpawnParams.Instigator = Cast<APawn>(SourceActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASMMonsterProjectile* Projectile = GetWorld()->SpawnActor<ASMMonsterProjectile>(
		ProjectileClass,
		SourceActor->GetActorLocation(),
		FireDirection.Rotation(),
		SpawnParams);

	if (Projectile)
	{
		Projectile->InitProjectile(SpecHandle, ProjectileRange, FireDirection, SourceActor);
	}

	// 정상 종료는 몽타주 완료/취소 콜백에서만 처리한다.
}

void UGA_MonsterRangedAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterRangedAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

float UGA_MonsterRangedAttack::GetMonsterAttackPower() const
{
	ASMMonsterBase* Monster = Cast<ASMMonsterBase>(GetAvatarActorFromActorInfo());
	if (Monster && Monster->MonsterAttributeSet)
	{
		return Monster->MonsterAttributeSet->GetAttackPower();
	}
	return 10.0f;
}
