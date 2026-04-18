#include "GAS/Abilities/GA_Explosion.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/SMPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GAS/Abilities/SkillActor/SMAExplosionCastActor.h"

UGA_Explosion::UGA_Explosion()
{
	ExplosionCastActorClass = ASMAExplosionCastActor::StaticClass();
}

void UGA_Explosion::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (LoadActiveSkillSummary(ActorInfo) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (ResolveTargetLocation(ActorInfo, ResolvedTargetLocation) == false)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	bCastFinished = false;
	bServerCastStarted = false;
	ActiveCastActor = nullptr;

	SetCasterMovementLocked(ActorInfo, true);

	UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
	if (AbilitySystemComponent == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (AttackMontage != nullptr)
	{
		AbilitySystemComponent->PlayMontage(this, ActivationInfo, AttackMontage, 1.0f);
	}

	UAbilityTask_WaitGameplayEvent* ReleaseEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		SMSkillTag::Event_Input_AttackReleased);
	ReleaseEventTask->EventReceived.AddDynamic(this, &ThisClass::OnAttackReleasedEventReceived);
	ReleaseEventTask->ReadyForActivation();

	APawn* AvatarPawn = Cast<APawn>(AvatarActor);
	if (AvatarPawn == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (AvatarPawn->IsLocallyControlled())
	{
		if (ResolveTargetLocation(ActorInfo, ResolvedTargetLocation) == false)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		CurrentAimOrigin = AvatarActor->GetActorLocation();
		CurrentTargetLocation = ResolvedTargetLocation;
		CurrentAimDirection = (ResolvedTargetLocation - CurrentAimOrigin).GetSafeNormal2D();

		if (AvatarActor->HasAuthority())
		{
			BeginServerCast(ActorInfo, ResolvedTargetLocation);
			return;
		}

		FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent, true);

		FGameplayAbilityTargetData_LocationInfo* LocationTargetData = new FGameplayAbilityTargetData_LocationInfo();
		LocationTargetData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationTargetData->SourceLocation.LiteralTransform = FTransform(CurrentAimOrigin);
		LocationTargetData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocationTargetData->TargetLocation.LiteralTransform = FTransform(CurrentTargetLocation);

		FGameplayAbilityTargetDataHandle TargetDataHandle;
		TargetDataHandle.Add(LocationTargetData);

		AbilitySystemComponent->ServerSetReplicatedTargetData(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey(),
			TargetDataHandle,
			FGameplayTag(),
			AbilitySystemComponent->ScopedPredictionKey);

		StartCastTimer(GetResolvedCastTime());
		return;
	}

	if (AvatarActor->HasAuthority())
	{
		AbilitySystemComponent->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()).AddUObject(
			this,
			&ThisClass::OnExplosionTargetDataReady);

		AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey());
	}
}

void UGA_Explosion::BeginServerCast(const FGameplayAbilityActorInfo* ActorInfo, const FVector& TargetLocation)
{
	if (IsActive() == false || bServerCastStarted)
	{
		return;
	}

	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UWorld* World = GetWorld();
	if (World == nullptr || ExplosionCastActorClass == nullptr)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	ResolvedTargetLocation = TargetLocation;
	CurrentAimOrigin = AvatarActor->GetActorLocation();
	CurrentTargetLocation = TargetLocation;
	CurrentAimDirection = (TargetLocation - CurrentAimOrigin).GetSafeNormal2D();

	bServerCastStarted = true;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AvatarActor;
	SpawnParams.Instigator = Cast<APawn>(AvatarActor);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveCastActor = World->SpawnActor<ASMAExplosionCastActor>(
		ExplosionCastActorClass,
		ResolvedTargetLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (ActiveCastActor == nullptr)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	ActiveCastActor->InitializeCast(
		MakeDamageSpec(ActorInfo),
		AvatarActor,
		GetResolvedCastTime(),
		GetResolvedExplosionRadius());

	StartCastTimer(GetResolvedCastTime());
}

void UGA_Explosion::StartCastTimer(float InCastTime)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CastTimerHandle);
	}

	if (InCastTime <= KINDA_SMALL_NUMBER)
	{
		HandleCastFinished();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CastTimerHandle,
			this,
			&ThisClass::HandleCastFinished,
			InCastTime,
			false);
	}
}

void UGA_Explosion::OnExplosionTargetDataReady(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ApplicationTag)
{
	if (IsActive() == false || bServerCastStarted)
	{
		return;
	}

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo == nullptr)
	{
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
	{
		AbilitySystemComponent->ConsumeClientReplicatedTargetData(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey());
	}

	if (TargetDataHandle.Num() <= 0)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	const FGameplayAbilityTargetData* TargetData = TargetDataHandle.Get(0);
	if (TargetData == nullptr)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	BeginServerCast(ActorInfo, TargetData->GetEndPoint());
}

void UGA_Explosion::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CastTimerHandle);
	}

	ASMAExplosionCastActor* CastActorToCleanup = ActiveCastActor.Get();
	if (CastActorToCleanup != nullptr && CastActorToCleanup->HasAuthority() && bCastFinished == false)
	{
		CastActorToCleanup->CancelCast();
	}

	if (ActorInfo != nullptr && ActorInfo->AbilitySystemComponent.IsValid())
	{
		UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get();
		AbilitySystemComponent->AbilityTargetDataSetDelegate(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()).RemoveAll(this);
		AbilitySystemComponent->ConsumeClientReplicatedTargetData(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey());
		AbilitySystemComponent->CurrentMontageStop();
	}

	SetCasterMovementLocked(ActorInfo, false);
	bServerCastStarted = false;
	ActiveCastActor = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Explosion::OnSkillEffect(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FVector& TargetLocation,
	const FVector& AimDirection)
{
	// Explosion은 ActivateAbility에서 전용 흐름으로 처리
}

void UGA_Explosion::HandleCastFinished()
{
	if (IsActive() == false)
	{
		return;
	}

	bCastFinished = true;

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (AvatarActor->HasAuthority() == false)
	{
		return;
	}

	if (CommitAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo()) == false)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, true);
		return;
	}

	if (ActiveCastActor != nullptr)
	{
		ActiveCastActor->TriggerExplosion();
	}

	EndAbility(GetCurrentAbilitySpecHandle(), ActorInfo, GetCurrentActivationInfo(), true, false);
}

void UGA_Explosion::OnAttackReleasedEventReceived(FGameplayEventData Payload)
{
	if (IsActive() == false)
	{
		return;
	}

	if (bCastFinished)
	{
		return;
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
}

bool UGA_Explosion::ResolveTargetLocation(const FGameplayAbilityActorInfo* ActorInfo, FVector& OutTargetLocation) const
{
	OutTargetLocation = FVector::ZeroVector;

	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false)
	{
		return false;
	}

	APawn* AvatarPawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (AvatarPawn == nullptr)
	{
		return false;
	}

	const FVector AvatarLocation = AvatarPawn->GetActorLocation();
	const float FixedTargetingRangeCm = FMath::Max(TargetingRangeCm, 0.0f);

	FVector MouseGroundLocation = FVector::ZeroVector;
	if (TryGetMouseGroundLocation(AvatarPawn, MouseGroundLocation))
	{
		const FVector ToTarget = MouseGroundLocation - AvatarLocation;
		const float Distance2D = ToTarget.Size2D();

		if (Distance2D <= KINDA_SMALL_NUMBER)
		{
			OutTargetLocation = AvatarLocation;
			OutTargetLocation.Z = MouseGroundLocation.Z;
			return true;
		}

		const FVector Direction2D = ToTarget.GetSafeNormal2D();
		const float ClampedDistance = FMath::Min(Distance2D, FixedTargetingRangeCm);

		OutTargetLocation = AvatarLocation + (Direction2D * ClampedDistance);
		OutTargetLocation.Z = MouseGroundLocation.Z;
		return true;
	}

	FVector FallbackDirection = AvatarPawn->GetActorForwardVector().GetSafeNormal2D();
	if (FallbackDirection.IsNearlyZero())
	{
		FallbackDirection = FVector::ForwardVector;
	}

	OutTargetLocation = AvatarLocation + (FallbackDirection * FixedTargetingRangeCm);
	OutTargetLocation.Z = AvatarLocation.Z;
	return true;
}

void UGA_Explosion::SetCasterMovementLocked(const FGameplayAbilityActorInfo* ActorInfo, bool bLocked) const
{
	if (ActorInfo == nullptr || ActorInfo->AvatarActor.IsValid() == false)
	{
		return;
	}

	ASMPlayerCharacter* AvatarCharacter = Cast<ASMPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (AvatarCharacter == nullptr)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(AvatarCharacter->GetController()))
	{
		PlayerController->SetIgnoreMoveInput(bLocked);
	}

	if (UCharacterMovementComponent* MovementComponent = AvatarCharacter->GetCharacterMovement())
	{
		if (bLocked)
		{
			MovementComponent->StopMovementImmediately();
			MovementComponent->DisableMovement();
		}
		else if (AvatarCharacter->IsDead() == false)
		{
			MovementComponent->SetMovementMode(MOVE_Walking);
		}
	}
}

float UGA_Explosion::GetResolvedCastTime() const
{
	float BaseChargeTime = FMath::Max(FixedCastTime, 0.0f);

	const float FinalDuration = FMath::Max(FieldDuration, 0.0f);
	const float ChargeTimeReduction = FMath::Max(FinalDuration - BaseChargeTime, 0.0f);

	return FMath::Max(BaseChargeTime - ChargeTimeReduction, 0.0f);
}

float UGA_Explosion::GetResolvedExplosionRadius() const
{
	return FMath::Max(RangeCm, 0.0f);
}
