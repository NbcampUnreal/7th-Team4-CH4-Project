#include "SMASkillField.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"

ASMASkillField::ASMASkillField()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 범위 콜리전
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComponent->InitBoxExtent(FieldBoxExtent);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASMASkillField::OnFieldBeginOverlap);
	CollisionComponent->OnComponentEndOverlap.AddDynamic(this, &ASMASkillField::OnFieldEndOverlap);
	SetRootComponent(CollisionComponent);
}

void ASMASkillField::BeginPlay()
{
	Super::BeginPlay();
}


void ASMASkillField::InitField(FGameplayEffectSpecHandle InSpecHandle,
                               AActor* InInstigatorActor,
                               float InDuration)
{
	DamageSpecHandle = InSpecHandle;
	InstigatorActor = InInstigatorActor;
	Duration = InDuration;

	// 장판 지속 시간 종료 타이머
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DurationEndHandle,
			this,
			&ASMASkillField::OnDurationExpired,
			Duration,
			false
		);
	}

	// 콜리전 스폰 시점에 데미지를 줄 수 있도록
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

	Destroy();
}
