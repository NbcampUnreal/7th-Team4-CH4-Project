// AGCN_LineTraceChain.cpp


#include "GCN_LineTraceChain.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "GameFramework/Character.h"
#include "GameplayTags/Enemy/SMEnemyTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "GAS/SMGameplayAbilityUtils.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values
AGCN_LineTraceChain::AGCN_LineTraceChain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bAutoDestroyOnRemove = true;
}

bool AGCN_LineTraceChain::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	InitializeChain(MyTarget, Parameters);
	return true;
}

bool AGCN_LineTraceChain::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::WhileActive_Implementation(MyTarget, Parameters);
	InitializeChain(MyTarget, Parameters);
	return true;
}

bool AGCN_LineTraceChain::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	SetActorTickEnabled(false);

	for (UNiagaraComponent* Comp : ChainBeamComponents)
	{
		if (IsValid(Comp) == false) continue;
		Comp->Deactivate();
		Comp->DestroyComponent();
	}
	ChainBeamComponents.Empty();
	
	return Super::OnRemove_Implementation(MyTarget, Parameters);
}

void AGCN_LineTraceChain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateChain();
}

void AGCN_LineTraceChain::InitializeChain(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	// 이미 초기화되어 있으면 스킵
	if (ChainBeamComponents.Num() > 0) return;

	OwnerActor = MyTarget;
	BeamRange = Parameters.RawMagnitude;
	ChainSearchRadius = BeamRange * ChainSearchRadiusMultiplier;
	MaxChainCount = FMath::Max(1, FMath::RoundToInt(Parameters.NormalizedMagnitude));

	if (IsValid(ChainBeamNiagaraSystem) == false)
	{
		UE_LOG(LogTemp,Warning,TEXT("ChainBeamNiagaraSystem is Not valid"))
		return;
	}
	// MaxChainCount개의 컴포넌트 생성 (구간 수 = 체인 횟수)
	// 예) MaxChainCount=3 → [시전자→적1], [적1→적2], [적2→적3]
	for (int32 i = 0; i < MaxChainCount; ++i)
	{
		UNiagaraComponent* Comp = NewObject<UNiagaraComponent>(this);
		Comp->SetAsset(ChainBeamNiagaraSystem);
		Comp->SetAutoActivate(false);
		Comp->AttachToComponent(GetRootComponent(),
								FAttachmentTransformRules::KeepWorldTransform);
		Comp->RegisterComponent();
		ChainBeamComponents.Add(Comp);
	}

	SetActorTickEnabled(true);
}

void AGCN_LineTraceChain::UpdateChain()
{
	if (OwnerActor.IsValid() == false) return;
	
	ACharacter* Character = Cast<ACharacter>(OwnerActor.Get());
	if (Character == nullptr) return;
	
	//시전자 소켓 위치 및 조준 방향
	FVector Origin = GetAttachSocketLocation(Character);
	
	FVector AimDirection = Character->GetBaseAimRotation().Vector();
	
	AController* Controller = Character->GetController();
	if (IsValid(Controller) == true)
	{
		//스킬 사용플레이어인 경우 Controller 방향으로
		AimDirection = Controller->GetControlRotation().Vector();
	}
	
	//LineTrace로 첫 번째 적 탐색
	const FVector TraceEnd = Origin + AimDirection*BeamRange;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor.Get());
	
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults,Origin,TraceEnd,ECC_Pawn,Params);
	
	//체인 포인트 배열 구성
	TArray<FVector> ChainPoints;
	TArray<AActor*> ChainedActors;
	ChainedActors.Add(OwnerActor.Get()); // 시전자 탐색 제외
	
	//첫 번째 적 탐색
	AActor* FirstEnemy = nullptr;
	for (const auto& Hit: HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (IsValid(HitActor) == false) continue;
		if (SMGameplayAbilityUtils::HasTeamTag(HitActor) == true) continue;
		if (SMGameplayAbilityUtils::IsAvailableEnemy(HitActor) == false) continue;
		FirstEnemy = HitActor;
		break;
	}
	
	ChainPoints.Add(Origin);
	
	if (IsValid(FirstEnemy) == true)
	{
		ChainPoints.Add(FirstEnemy->GetActorLocation());
		ChainedActors.Add(FirstEnemy);
		
		for (int32 i = 1; i < MaxChainCount; i++)
		{
			AActor* NextEnemy = nullptr;
			if (FindNearestEnemy(ChainPoints.Last(),ChainedActors,NextEnemy) == false) break;
			
			ChainPoints.Add(NextEnemy->GetActorLocation());
			ChainedActors.Add(NextEnemy);
		}
	}
	else
	{
		//첫 적도 없으면 사거리 끝까지 빔 1개만
		ChainPoints.Add(TraceEnd);
	}
	
	// 구간 수 = ChainPoints.Num() -1
	const int32 SegmentCount = ChainPoints.Num() - 1;
	
	//각 컴포넌트 위치 및 Beam End 갱신
	for (int32 i = 0; i < ChainBeamComponents.Num(); i++)
	{
		UNiagaraComponent* Comp = ChainBeamComponents[i];
		if (IsValid(Comp) == false) continue;
		
		if ( i < SegmentCount)
		{
			if (Comp->IsActive() == false)
			{
				Comp->Activate(true);
			}
			Comp->SetWorldLocation(ChainPoints[i]);
			
			Comp->SetVariableVec3(TEXT("BeamEnd"), ChainPoints[i+1]);
		}
		else
		{
			//빔이 닿지 않음 -> 비활성화
			if (Comp->IsActive() == true) Comp->Deactivate();
		}
	}
}

bool AGCN_LineTraceChain::FindNearestEnemy(const FVector& Origin, const TArray<AActor*>& ExcludeActors,
	AActor*& OutEnemy) const
{
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OverlapActors;
	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Origin,
		ChainSearchRadius,
		ObjectTypes,
		nullptr,        // 필터 클래스 없음, HasAnyTeamTag로 아군 제외
		ExcludeActors,  // 시전자 + 이미 체인된 적 자동 제외
		OverlapActors
	);

	float NearestDistSq = FLT_MAX;

	for (AActor* Actor : OverlapActors)
	{
		if (IsValid(Actor) == false) continue;
		if (SMGameplayAbilityUtils::HasTeamTag(Actor) == true) continue;
		if (SMGameplayAbilityUtils::IsAvailableEnemy(Actor) == false) continue;

		const float DistSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			OutEnemy = Actor;
		}
	}
	return IsValid(OutEnemy);
}

FVector AGCN_LineTraceChain::GetAttachSocketLocation(ACharacter* Character) const
{
	//  WeaponSocket에 붙은 StaticMesh에서 탐색
	TArray<UStaticMeshComponent*> Comps;
	Character->GetComponents<UStaticMeshComponent>(Comps);
	for (UStaticMeshComponent* Comp : Comps)
	{
		if (IsValid(Comp) == false) continue;
		if (Comp->GetAttachSocketName() != FName("WeaponSocket")) continue;
		if (Comp->DoesSocketExist(AttachSocketName))
		{
			return Comp->GetSocketLocation(AttachSocketName);
		}
		break;
	}

	// 못 찾으면 캐릭터 루트 위치 반환
	return Character->GetActorLocation();
}


