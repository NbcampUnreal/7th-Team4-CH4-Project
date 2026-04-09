#include "GA_SkillField.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "SkillActor/SMASkillField.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UGA_SkillField::UGA_SkillField()
{
	FieldClass = ASMASkillField::StaticClass();
}

void UGA_SkillField::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
	// 데디케이트 서버는 HasAuthority()=true 이라 건너뜀
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get());
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

		//클라에서만 실행 마우스 위치는 클라만
		if (Pawn && ASC && !Pawn->HasAuthority())
		{
			//마우스가 가리키는 좌표 계산
			FVector MouseGroundLocation = Pawn->GetActorLocation();
			if (TryGetMouseGroundLocation(Pawn, MouseGroundLocation))
			{
				// 타겟 데이터 생성 및 패킹
				FGameplayAbilityTargetData_LocationInfo* LocData = new FGameplayAbilityTargetData_LocationInfo();
				//좌표 타입을 LifealTransform으로 설정하고 location 집어넣기
				LocData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
				LocData->TargetLocation.LiteralTransform = FTransform(MouseGroundLocation);
				
				//핸들에 생성한 데이터 ADD
				FGameplayAbilityTargetDataHandle TargetHandle;
				TargetHandle.Add(LocData);

				// 서버로 데이터를 복제해서 전송
				ASC->ServerSetReplicatedTargetData(
					Handle,
					ActivationInfo.GetActivationPredictionKey(),
					TargetHandle,
					FGameplayTag(),
					ASC->ScopedPredictionKey
				);
			}
		}
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_SkillField::OnSkillEffect(const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = ActorInfo && ActorInfo->AvatarActor.IsValid()
		                ? Cast<APawn>(ActorInfo->AvatarActor.Get())
		                : nullptr;

	if (!Avatar)
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			ActorInfo,
			GetCurrentActivationInfo(),
			true,
			true);
		return;
	}

	if (!Avatar->HasAuthority())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC)
		{
			FVector SpawnLocation = Avatar->GetActorLocation();
			//마우스 위치 계산한거 파라미터로
			TryGetMouseGroundLocation(Avatar, SpawnLocation);

			FGameplayCueParameters CueParams;
			CueParams.Location = SpawnLocation;
			//클라에서 장판 이펙트 생성
			ASC->AddGameplayCue(SMSkillTag::GameplayCue_Skill_SpawnField_Tick, CueParams);
		}
		//클라에서는 여기까지 수행하고 종료 이후는 서버가 처리하게
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			ActorInfo,
			GetCurrentActivationInfo(),
			true,
			true);
		return;
	}

	if (!GetWorld() || !FieldClass)
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			ActorInfo,
			GetCurrentActivationInfo(),
			true,
			true);
		return;
	}

	// 데디서버에서 클라가 보낸 타겟데이터 수신
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		EndAbility(
			GetCurrentAbilitySpecHandle(),
			ActorInfo,
			GetCurrentActivationInfo(),
			true,
			true);
		return;
	}

	//타켓 데이터 델리게이트 설정 - 서버
	SourceASC->AbilityTargetDataSetDelegate(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	).AddUObject(this, &UGA_SkillField::OnTargetDataReady);

	// 델리가 먼저 도착하면 즉시 델리게이트 실행 아니면 예측부터
	SourceASC->CallReplicatedTargetDataDelegatesIfSet(
		GetCurrentAbilitySpecHandle(),
		GetCurrentActivationInfo().GetActivationPredictionKey()
	);
}

void UGA_SkillField::OnTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag)
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	if (!Avatar) return;

	// 타겟데이터에서 클라이언트가 보낸 스폰 위치확인
	FVector SpawnLocation = Avatar->GetActorLocation();
	if (TargetData.Num() > 0)
	{
		if (const FGameplayAbilityTargetData* Data = TargetData.Get(0))
		{
			SpawnLocation = Data->GetEndPoint();
		}
	}

	//장판 스폰
	SpawnFieldAtLocation(SpawnLocation, ActorInfo);
	EndAbility(
		GetCurrentAbilitySpecHandle(),
		ActorInfo,
		GetCurrentActivationInfo(),
		true,
		false);
}

void UGA_SkillField::SpawnFieldAtLocation(const FVector& SpawnLocation, const FGameplayAbilityActorInfo* ActorInfo)
{
	APawn* Avatar = Cast<APawn>(ActorInfo->AvatarActor.Get());
	UWorld* World = GetWorld();
	if (!Avatar || !World || !FieldClass) return;

	FGameplayEffectSpecHandle SpecHandle = MakeDamageSpec(ActorInfo);
	if (!SpecHandle.IsValid()) return;

	FActorSpawnParameters Params;
	Params.Owner = Avatar;
	Params.Instigator = Avatar;
	//스폰된 자리에 다른물체가 있어도 스폰되게
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASMASkillField* Field = World->SpawnActor<ASMASkillField>(
		FieldClass, SpawnLocation, FRotator::ZeroRotator, Params);

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (Field)
	{
		Field->InitField(SpecHandle, Avatar, FieldDuration);

		if (SourceASC)
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = SpawnLocation;
			CueParams.RawMagnitude = FieldDuration;
			SourceASC->AddGameplayCue(SMSkillTag::GameplayCue_Skill_SpawnField_Tick, CueParams);
		}
	}
}

bool UGA_SkillField::TryGetMouseGroundLocation(APawn* Pawn, FVector& OutLocation) const
{
	if (!Pawn) return false;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return false;

	FVector MouseLoc, MouseDir;
	if (!PC->DeprojectMousePositionToWorld(MouseLoc, MouseDir)) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Pawn);

	if (World->LineTraceSingleByChannel(
		Hit,
		MouseLoc,
		MouseLoc + MouseDir * 100000.f,
		ECC_Visibility,
		QueryParams))
	{
		OutLocation = Hit.ImpactPoint;
		return true;
	}
	return false;
}
