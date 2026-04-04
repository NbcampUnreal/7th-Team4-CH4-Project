#include "Components/SMInteractionTargetComponent.h"

#include "Components/MeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Test/InteractionTestActor.h"

USMInteractionTargetComponent::USMInteractionTargetComponent()
	: bInteractionEnabled(true)
	  , InteractionPriority(0)
	  , HighlightOverlayMaterial(nullptr)
	  , bAutoCollectOwnerMeshComponents(true)
	  , bFocusedLocally(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USMInteractionTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoCollectOwnerMeshComponents && HighlightMeshComponents.Num() == 0)
	{
		CollectOwnerMeshComponents();
	}

	ApplyHighlightState();
}

void USMInteractionTargetComponent::SetInteractionEnabledRuntime(bool bInInteractionEnabled)
{
	bInteractionEnabled = bInInteractionEnabled;

	if (bInteractionEnabled == false)
	{
		ClearLocalFocus();
	}
}

void USMInteractionTargetComponent::SetFocusedLocally(bool bInFocusedLocally)
{
	if (bFocusedLocally == bInFocusedLocally)
	{
		return;
	}

	bFocusedLocally = bInFocusedLocally;
	ApplyHighlightState();
}

void USMInteractionTargetComponent::ClearLocalFocus()
{
	SetFocusedLocally(false);
}

void USMInteractionTargetComponent::Interact(APawn* InInteractingPawn)
{
	if (CanInteract(InInteractingPawn) == false)
	{
		return;
	}

	HandleOwnerInteract(InInteractingPawn);
}

bool USMInteractionTargetComponent::CanInteract(APawn* InInteractingPawn) const
{
	if (bInteractionEnabled == false)
	{
		return false;
	}

	if (GetOwner() == nullptr)
	{
		return false;
	}

	return true;
}

void USMInteractionTargetComponent::HandleOwnerInteract(APawn* InInteractingPawn)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	/**
	 * TODO:
	 * 여기에서 GetOwner() 기준 else if 분기로 실제 상호작용을 연결합니다.
	 *
	 * 예시:
	 * if (ASMBaseItemDropActor* DropActor = Cast<ASMBaseItemDropActor>(OwnerActor))
	 * {
	 *     DropActor->HandleInteract(InInteractingPawn);
	 * }
	 * else if (ASMShopNpc* ShopNpc = Cast<ASMShopNpc>(OwnerActor))
	 * {
	 *     ShopNpc->HandleInteract(InInteractingPawn);
	 * }
	 * else if (ASMInteractableBuilding* Building = Cast<ASMInteractableBuilding>(OwnerActor))
	 * {
	 *     Building->HandleInteract(InInteractingPawn);
	 * }
	 */
	
	// TestActor용 테스트 코드
	if (AInteractionTestActor* TestActor = Cast<AInteractionTestActor>(OwnerActor))
	{
		TestActor->TestInteract(InInteractingPawn);
	}
}

void USMInteractionTargetComponent::ApplyHighlightState()
{
	const bool bApplyHighlight =
		bInteractionEnabled &&
		bFocusedLocally &&
		HighlightOverlayMaterial != nullptr;

	for (UMeshComponent* MeshComponent : HighlightMeshComponents)
	{
		ApplyHighlightToMesh(MeshComponent, bApplyHighlight);
	}
}

void USMInteractionTargetComponent::ApplyHighlightToMesh(UMeshComponent* InMeshComponent, bool bInApplyHighlight) const
{
	if (InMeshComponent == nullptr)
	{
		return;
	}

	InMeshComponent->SetOverlayMaterial(bInApplyHighlight ? HighlightOverlayMaterial : nullptr);
}

void USMInteractionTargetComponent::CollectOwnerMeshComponents()
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	TArray<UMeshComponent*> FoundMeshComponents;
	OwnerActor->GetComponents<UMeshComponent>(FoundMeshComponents);

	for (UMeshComponent* MeshComponent : FoundMeshComponents)
	{
		if (MeshComponent == nullptr)
		{
			continue;
		}

		HighlightMeshComponents.AddUnique(MeshComponent);
	}
}
