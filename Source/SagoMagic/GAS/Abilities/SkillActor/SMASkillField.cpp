#include "SMASkillField.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "Building/SMBaseCampActor.h"
#include "Building/SMBaseBuilding.h"
#include "GameplayTags/Character/SMSkillTag.h"

ASMASkillField::ASMASkillField()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 범위 콜리전
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->InitSphereRadius(FieldRadius);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASMASkillField::OnFieldBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ASMASkillField::OnFieldEndOverlap);
	SetRootComponent(CollisionComponent);
}

void ASMASkillField::BeginPlay()
{
	Super::BeginPlay();
}


void ASMASkillField::InitField(
	FGameplayEffectSpecHandle InSpecHandle,
	AActor* InInstigatorActor,
	float InDuration,
	float InRangeCm,
	bool bEnablePull,
	bool bEnableSlow)
{
	DamageSpecHandle = InSpecHandle;
	InstigatorActor = InInstigatorActor;
	Duration = InDuration;
	bSlowEnabled = bEnableSlow;

	if (InRangeCm > 0.f && CollisionComponent)
	{
		CollisionComponent->SetSphereRadius(InRangeCm);
	}

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InInstigatorActor);
	ActiveCueTag = SMSkillTag::GameplayCue_Skill_SpawnField_Tick;

	
	
	// 서버에서 Cue - GA의 예측키 스코프 밖이므로 모든 클라에 복제
	// 서버가 Owner 클라에 대한 전송을 스킵하는 경우가 있음
	// 명시적으로 예측키를 지움으로 local에 항상 복제
	// RemoveGameplayCue는 OnDurationExpired에서 처리
	if (HasAuthority() && IsValid(OwnerASC) && CueEffectClass)
	{
		FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
		Context.AddOrigin(GetActorLocation());
		Context.AddInstigator(this, this);
		
		FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(CueEffectClass, 1.0f, Context);
		
		if (Spec.IsValid())
		{
			CueEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// 장판 지속 시간 종료 타이머
	World->GetTimerManager().SetTimer(
		DurationEndHandle,
		this,
		&ASMASkillField::OnDurationExpired,
		Duration,
		false
	);

	// 스폰 직후 즉시 데미지 방지 딜레이 후 그 시점 오버랩 액터만 처리
	World->GetTimerManager().SetTimer(
		InitialOverlapHandle,
		this,
		&ASMASkillField::CheckInitialOverlaps,
		StartDelay,
		false
	);
}

void ASMASkillField::CheckInitialOverlaps()
{
	if (!HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	CollisionComponent->GetOverlappingActors(OverlappingActors);
	for (AActor* Actor : OverlappingActors)
	{
		OnFieldBeginOverlap(
			CollisionComponent,
			Actor,
			nullptr,
			0,
			false,
			FHitResult());
	}
}

void ASMASkillField::OnFieldBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                         AActor* OtherActor,
                                         UPrimitiveComponent* OtherComponent,
                                         int32 OtherBodyIndex,
                                         bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor) return;
	// 시전자가 죽어도 장판은 계속 동작
	if (InstigatorActor.IsValid() && OtherActor == InstigatorActor.Get()) return;

	// 아군 구조물 통과 베이스캠프, 건축물
	if (OtherActor->IsA<ASMBaseCampActor>()) return;
	if (OtherActor->IsA<ASMBaseBuilding>()) return;

	// 팀 태그 공격x
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC && TargetASC->HasMatchingGameplayTag(SMGameFlowTag::Team)) return;

	if (!TargetASC || !DamageSpecHandle.IsValid()) return;

	// 이미 GE가 적용된 액터는 중복 적용 방지
	if (ActiveEffectHandles.Contains(OtherActor)) return;

	// GE 적용 후 핸들 저장 EndOverlap 시 제거용
	FActiveGameplayEffectHandle EffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	if (EffectHandle.IsValid())
	{
		ActiveEffectHandles.Add(OtherActor, EffectHandle);
	}

	// 슬로우 적용
	if (bSlowEnabled)
	{
		ApplySlow(OtherActor);
	}
}

//콜리전에서 몬스터가 밖으로 나갔을때 GE제거
void ASMASkillField::OnFieldEndOverlap(UPrimitiveComponent* OverlappedComponent,
                                       AActor* OtherActor,
                                       UPrimitiveComponent* OtherComponent,
                                       int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;
	if (!OtherActor) return;

	if (FActiveGameplayEffectHandle* Handle = ActiveEffectHandles.Find(OtherActor))
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			TargetASC->RemoveActiveGameplayEffect(*Handle);
		}
		ActiveEffectHandles.Remove(OtherActor);
	}

	// 슬로우 해제
	RestoreSpeed(OtherActor);
}

void ASMASkillField::OnDurationExpired()
{
	if (!HasAuthority()) return;

	// 콜리전 비활성화 더 이상 새로운 피해 없게
	if (CollisionComponent)
	{
		CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 범위 내 액터들의 GE 전부 제거
	for (auto& Pair : ActiveEffectHandles)
	{
		if (!IsValid(Pair.Key)) continue;
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pair.Key);
		if (TargetASC)
		{
			TargetASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}
	ActiveEffectHandles.Empty();

	// 슬로우 전체 해제
	for (auto& Pair : OriginalMoveSpeeds)
	{
		RestoreSpeed(Pair.Key);
	}
	OriginalMoveSpeeds.Empty();

	if (IsValid(OwnerASC))
	{
		OwnerASC->RemoveActiveGameplayEffect(CueEffectHandle);
	}

	Destroy();
}


void ASMASkillField::ApplyPull()
{
	if (!HasAuthority()) return;

	TArray<AActor*> OverlappingActors;
	CollisionComponent->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (!IsValid(Actor)) continue;
		if (InstigatorActor.IsValid() && Actor == InstigatorActor.Get()) continue;
		if (Actor->IsA<ASMBaseCampActor>() || Actor->IsA<ASMBaseBuilding>()) continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC->HasMatchingGameplayTag(SMGameFlowTag::Team)) continue;

		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character) continue;

		FVector PullDir = (GetActorLocation() - Actor->GetActorLocation()).GetSafeNormal2D();
		Character->LaunchCharacter(PullDir * PullStrength, true, false);
	}
}

void ASMASkillField::ApplySlow(AActor* Actor)
{
	if (!IsValid(Actor)) return;
	if (OriginalMoveSpeeds.Contains(Actor)) return; // 이미 슬로우 중

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (!Character) return;

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	if (!Movement) return;

	// 원래 속도 저장 후 감소
	OriginalMoveSpeeds.Add(Actor, Movement->MaxWalkSpeed);
	Movement->MaxWalkSpeed *= SlowMultiplier;
}

void ASMASkillField::RestoreSpeed(AActor* Actor)
{
	if (!IsValid(Actor)) return;

	float* OriginalSpeed = OriginalMoveSpeeds.Find(Actor);
	if (!OriginalSpeed) return;

	ACharacter* Character = Cast<ACharacter>(Actor);
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = *OriginalSpeed;
	}
	OriginalMoveSpeeds.Remove(Actor);
}
