#include "GAS/Abilities/GA_SkillBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "GameplayEffectTypes.h"
#include "SagoMagic.h"

#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UGA_SkillBase::UGA_SkillBase()
{
	//어빌리티 인스턴스 생성 - 단 하나
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	//어빌리티가 네트워크 어디서 실행될지 - 클라이언트
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	//인스턴스를 네트워크로 복제할지 - 복제안함
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
}

bool UGA_SkillBase::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	//GAS의 기본조건을 자동을 체크 (Ability, ActivationBlockedTags 등)
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_SkillBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 기존 - LoadSkillStats에서 DT 읽기
	// 변경 - 인벤에서 미리 계산된 최종값 받아오기
	if (!LoadActiveSkillSummary(ActorInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}


	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ExecuteSkillLogic(ActorInfo);
}

void UGA_SkillBase::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//CooldownGameplayEffectClass가 없거나 쿨다운이 0이면 패스
	if (!CooldownGameplayEffectClass || CooldownSeconds <= 0.f)
	{
		Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		return;
	}

	//GE Spec 생성
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
		CooldownGameplayEffectClass,
		GetAbilityLevel());
	if (!SpecHandle.IsValid()) return;

	//SetByCaller로 쿨다운 시간 주입.
	SpecHandle.Data->SetSetByCallerMagnitude(SMSkillTag::Data_Cooldown, CooldownSeconds);

	//스킬별 쿨다운 태그 추가
	//GA에 ActivationBlockTags에 같은 태그를 등록하면 쿨다운 중 스킬 발동 불가하는 식으로
	if (CooldownTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
	}

	//동적으로 쿨다운을 적용시켜주는 함수
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}


FString UGA_SkillBase::GetCurrentPredictionKeyStatus()
{
	//1. 현재 사용 중인 Prediction Key에 대한 번호를 가져옴
	//2. 이 키로 아직 더 예측을 더 할 수 있는가를 확인하고 물어보고 맞으면 true, 번호가 틀리거나 아니면 false를 반환
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (
		ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool UGA_SkillBase::IsPredictionKeyValidForMorePrediction() const
{
	//스킬을 방동하는 순간 Prediction Key가 생성됨 이하 예측 키, 또한 Ability Prediction Window - 유효시간이 생김.
	//예측 키는 어빌리티 활성화-> 타겟 데이터 전송 -> 코스메틱 효과 등을 한번에 처리 이것이 More Prediction
	//Prediction Key 만료 - 너무 많은 행동을 하면 서버에서 Acknowledgment하고 되서 만료가 되버림
	//Prediction Key가 만료가 되면 IsValidForMorePrediction()에서 false를 반환하고
	//새로운 Prediction Key를 생성하거나 서버의 허락을 기다려야함.
	//현재 예측 키 가 추가적인 예측을 더 할 수 있는 상태인지 bool값으로 반환.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

FGameplayEffectSpecHandle UGA_SkillBase::MakeDamageSpec(const FGameplayAbilityActorInfo* ActorInfo) const
{
	//시전자 정보, 적용할 GE 존재 유효성 검사
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid() || !DamageEffectClass)
	{
		return FGameplayEffectSpecHandle();
	}
	//시전자의 ASC가져오기
	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	if (!SourceASC) return FGameplayEffectSpecHandle();

	//시전자와 컨트롤러 정보 가져오기
	AActor* Avatar = ActorInfo->AvatarActor.Get();
	AController* Controller = nullptr;
	if (APawn* Pawn = Cast<APawn>(Avatar))
	{
		Controller = Pawn->GetController();
	}

	//누가 쐈는지 기록
	FGameplayEffectContextHandle ContextHandle = SourceASC->MakeEffectContext();
	ContextHandle.AddInstigator(Avatar, Controller);

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, ContextHandle);
	if (!SpecHandle.IsValid()) return SpecHandle;

	SpecHandle.Data->SetSetByCallerMagnitude(SMSkillTag::Data_Damage_Amount, -BaseDamage);
	return SpecHandle;
}


bool UGA_SkillBase::LoadActiveSkillSummary(const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		return false;
	}

	const APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
	const APlayerState* PS = Pawn ? Pawn->GetPlayerState() : nullptr;
	const USMInventoryComponent* InventoryComp =
		PS ? PS->FindComponentByClass<USMInventoryComponent>() : nullptr;

	if (!InventoryComp)
	{
		return false;
	}

	FSMCompiledSkillSummary ActiveSummary;
	if (!InventoryComp->GetActiveSkillSummary(ActiveSummary))
	{
		return false;
	}

	CachedSummary = ActiveSummary;
	BaseDamage = CachedSummary.GetFinalDamage();
	RangeCm = CachedSummary.GetFinalRangeOrArea();
	CooldownSeconds = CachedSummary.GetFinalCooldown();
	FieldDuration = CachedSummary.GetFinalDuration();
	TickInterval = CachedSummary.GetFinalTickInterval();
	SkillUpgradeTags  = CachedSummary.GetBehaviorTags();
	return true;
}

void UGA_SkillBase::ExecuteSkillLogic(const FGameplayAbilityActorInfo* ActorInfo)
{
	// 서버가 데이터를 받기전의 멤버변수를 사용하지 않게 데이터 초기화
	CurrentAimOrigin = FVector::ZeroVector;
	CurrentAimDirection = FVector::ForwardVector;
	CurrentTargetLocation = FVector::ZeroVector;

	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!Avatar || !ASC) return;

	// 서버는 스킬이 시작되자마자 델리게이트 생성
	if (Avatar->HasAuthority() && !Avatar->IsLocallyControlled())
	{
		ASC->AbilityTargetDataSetDelegate(GetCurrentAbilitySpecHandle(),
		                                  GetCurrentActivationInfo().GetActivationPredictionKey()
		).AddUObject(this, &UGA_SkillBase::OnTargetDataReadyCallBack);

		ASC->CallReplicatedTargetDataDelegatesIfSet(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey()
		);
	}

	StartMontageAndTasks();
}

void UGA_SkillBase::StartMontageAndTasks()
{
	// 몽타주 없으면 바로 스킬 실행
	if (!AttackMontage)
	{
		OnFireEventReceived(FGameplayEventData());
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
		return;
	}

	if (!SkillEventTag.IsValid())
	{
		SM_LOG(this, LogSM, Error, TEXT("SkillEventTag 미설정."));
		OnFireEventReceived(FGameplayEventData());
	}
	else
	{
		// 이벤트 태그 대기 Task
		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, SkillEventTag);
		EventTask->EventReceived.AddDynamic(this, &ThisClass::OnFireEventReceived);
		EventTask->ReadyForActivation();
	}
	

	// 몽타주 실행 Task
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->ReadyForActivation();
}

void UGA_SkillBase::OnFireEventReceived(FGameplayEventData Payload)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (!Avatar || !ASC) return;

	// Notify시점에 마우스 좌표 획득 및 서버 전송
	if (Avatar->IsLocallyControlled())
	{
		// Target데이터 보낼 때는 반드시 새로운 클라이언트 예측 창열기
		FScopedPredictionWindow ScopedPredictionWindow(ASC, true);
		FVector MouseLocation;


		if (!TryGetMouseGroundLocation(Avatar, MouseLocation))
		{
			MouseLocation = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * 1000.0f;
		}
		
		// 클라 데이터 갱신
		CurrentTargetLocation = MouseLocation;
		CurrentAimOrigin = Avatar->GetActorLocation();
		FVector Direction = (CurrentTargetLocation - CurrentAimOrigin).GetSafeNormal2D();
		CurrentAimDirection = Direction;

		// 마우스 좌표 포장
		FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
		LocData->SourceLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocData->SourceLocation.LiteralTransform = FTransform(CurrentAimOrigin);
		LocData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
		LocData->TargetLocation.LiteralTransform = FTransform(CurrentTargetLocation);

		FGameplayAbilityTargetDataHandle TargetHandle;
		TargetHandle.Add(LocData);

		// 서버로 전송
		ASC->ServerSetReplicatedTargetData(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey(),
			TargetHandle,
			FGameplayTag(),
			ASC->ScopedPredictionKey
		);

		// 클라는 즉시 실행
		OnSkillEffect(ActorInfo, CurrentTargetLocation, CurrentAimDirection);
	}
}

void UGA_SkillBase::OnTargetDataReadyCallBack(
	const FGameplayAbilityTargetDataHandle& TargetDataHandle,
	FGameplayTag ApplicationTag)
{
	// 이미 종료된 어빌리타가 늦게 온 데이터를 처리하지 않도록 방어
	if (!IsActive()) return;

	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo) return;

	// TargetData 메모리 정리
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->ConsumeClientReplicatedTargetData(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey());
	}
	

	if (TargetDataHandle.Num() > 0)
	{
		if (const FGameplayAbilityTargetData* Data = TargetDataHandle.Get(0))
		{
			// 서버 데이터 갱신
			CurrentTargetLocation = Data->GetEndPoint();
			CurrentAimOrigin = GetAvatarActorFromActorInfo()->GetActorLocation();
			CurrentAimDirection = (CurrentTargetLocation - CurrentAimOrigin).GetSafeNormal2D();

			// 최신 데이터가 확인되면 실행
			OnSkillEffect(ActorInfo, CurrentTargetLocation, CurrentAimDirection);
		}
	}

	if (!AttackMontage)
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
	}
}

void UGA_SkillBase::OnMontageFinished()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

bool UGA_SkillBase::TryGetMouseGroundLocation(APawn* Pawn, FVector& OutLocation) const
{
	if (!Pawn) return false;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return false;

	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, Hit))
	{
		OutLocation = Hit.Location;
		return true;
	}
	return false;
}
