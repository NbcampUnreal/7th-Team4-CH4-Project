// SMInteractionScannerComponent.cpp


#include "SMInteractionScannerComponent.h"

#include "Components/SMInteractionTargetComponent.h"


USMInteractionScannerComponent::USMInteractionScannerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 기본 스캔 반경(에디터 수정 가능)
	InitSphereRadius(300.0f);
	// 충돌 채널 세팅(에디터 수정 가능)
	SetCollisionProfileName(TEXT("Trigger"));
	
	bShowDebug = true;
}

void USMInteractionScannerComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnScannerBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnScannerEndOverlap);
}

void USMInteractionScannerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;
	
	// 디버그용
	if (bShowDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			GetComponentLocation(),
			GetScaledSphereRadius(),
			16,
			FColor::Green,
			false,
			-1.0f,
			0,
			1.5f
		);
	}

	// 배열이 비어 있으면 불끄고 종료
	if (OverlappedTargets.IsEmpty())
	{
		if (CurrentFocusedTarget)
		{
			CurrentFocusedTarget->SetFocusedLocally(false);
			CurrentFocusedTarget = nullptr;
		}
		
		return;
	}
	
	USMInteractionTargetComponent* ClosestTarget = FindClosestTarget();
	
	// 1등이 바뀔때 변경 로직
	if (ClosestTarget != CurrentFocusedTarget)
	{
		// 전 1등 불끄기
		if (CurrentFocusedTarget)
		{
			CurrentFocusedTarget->SetFocusedLocally(false);
		}
		
		// 새로운 1등 불 켜기
		if (ClosestTarget)
		{
			ClosestTarget->SetFocusedLocally(true);
		}
		
		CurrentFocusedTarget = ClosestTarget;
	}
}

void USMInteractionScannerComponent::OnScannerBeginOverlap(
	UPrimitiveComponent* OverlapComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSeep,
	const FHitResult& SweepResult)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;
	
	if (!OtherActor || OtherActor == GetOwner()) return;
	
	USMInteractionTargetComponent* TargetComp = OtherActor->FindComponentByClass<USMInteractionTargetComponent>();
	if (TargetComp)
	{
		OverlappedTargets.AddUnique(TargetComp);
	}
}

void USMInteractionScannerComponent::OnScannerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;
	
	if (!OtherActor) return;
	
	USMInteractionTargetComponent* TargetComp = OtherActor->FindComponentByClass<USMInteractionTargetComponent>();
	if (TargetComp)
	{
		OverlappedTargets.Remove(TargetComp);
	}
}

USMInteractionTargetComponent* USMInteractionScannerComponent::FindClosestTarget() const
{
	USMInteractionTargetComponent* BestTarget = nullptr;
	float MinDistanceSquared = MAX_FLT;
	FVector MyLocation = GetComponentLocation();
	
	for (USMInteractionTargetComponent* Target : OverlappedTargets)
	{
		if (!IsValid(Target)) continue;
		
		float DistSquared = FVector::DistSquared(MyLocation, Target->GetOwner()->GetActorLocation());
		
		if (DistSquared < MinDistanceSquared)
		{
			MinDistanceSquared = DistSquared;
			BestTarget = Target;
		}
	}
	
	return BestTarget;
}
