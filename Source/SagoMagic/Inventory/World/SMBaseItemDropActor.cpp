#include "Inventory/World/SMBaseItemDropActor.h"

#include "AssetDefinitionAssetInfo.h"
#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

#include "Components/SMInteractionTargetComponent.h"
#include "Inventory/Components/SMInventoryComponent.h"

#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMWorldVisualFragment.h"

ASMBaseItemDropActor::ASMBaseItemDropActor()
	: bInitialized(false)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootSceneComponent);

	InteractionTargetComponent = CreateDefaultSubobject<USMInteractionTargetComponent>(
		TEXT("InteractionTargetComponent"));

	if (InteractionTargetComponent != nullptr)
	{
		InteractionTargetComponent->SetInteractionDisplayText(FText::FromString(TEXT("습득")));
		InteractionTargetComponent->SetInteractionEnabled(false);
	}
}

void ASMBaseItemDropActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshInteractionState();
}

void ASMBaseItemDropActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMBaseItemDropActor, ItemDropPayload);
}

void ASMBaseItemDropActor::InitializeFromPayload(const FSMItemDropPayload& InItemDropPayload)
{
	ItemDropPayload = InItemDropPayload;
	bInitialized = HasValidPayload();

	if (bInitialized)
	{
		ApplyWorldVisual();
	}
	RefreshInteractionState();
}

void ASMBaseItemDropActor::HandleInteract(APawn* InInteractingPawn)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (InInteractingPawn == nullptr)
	{
		return;
	}

	if (HasValidPayload() == false)
	{
		return;
	}

	APlayerState* InteractingPlayerState = InInteractingPawn->GetPlayerState();
	if (InteractingPlayerState == nullptr)
	{
		return;
	}

	USMInventoryComponent* InventoryComponent = InInteractingPawn->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}
	
	const FGuid AddedItemInstanceId = InventoryComponent->AddItemFromDropPayload(ItemDropPayload);
	if (AddedItemInstanceId.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Guid for current item is invalid. can't get item from actor %s"), *InventoryComponent->GetName());
		return;
	}
	
	UE_LOG(LogTemp, Log, TEXT("Successfully added item from drop payload to inventory. ItemInstanceId: %s"), *AddedItemInstanceId.ToString());
	Destroy();

}

void ASMBaseItemDropActor::OnRep_ItemDropPayload()
{
	bInitialized = HasValidPayload();

	if (bInitialized)
	{
		ApplyWorldVisual();
	}

	RefreshInteractionState();
}

const USMItemDefinition* ASMBaseItemDropActor::ResolveItemDefinition() const
{
	if (ItemDropPayload.Definition.IsNull())
	{
		return nullptr;
	}

	return ItemDropPayload.Definition.LoadSynchronous();
}

void ASMBaseItemDropActor::ApplyWorldVisual()
{
	const USMItemDefinition* ItemDefinition = ResolveItemDefinition();
	if (ItemDefinition == nullptr)
	{
		return;
	}

	const USMWorldVisualFragment* WorldVisualFragment = FindWorldVisualFragment(ItemDefinition);
	if (WorldVisualFragment == nullptr)
	{
		return;
	}

	if (StaticMeshComponent == nullptr)
	{
		return;
	}

	/**
	 * TODO:
	 * WorldVisualFragment에서
	 * WorldMesh / OverrideMaterial / WorldScale을 읽어
	 * StaticMeshComponent에 적용합니다.
	 */
}

void ASMBaseItemDropActor::RefreshInteractionState()
{
	if (InteractionTargetComponent == nullptr)
	{
		return;
	}

	InteractionTargetComponent->SetInteractionEnabledRuntime(HasValidPayload());
}

bool ASMBaseItemDropActor::HasValidPayload() const
{
	return ItemDropPayload.IsValidPayload();
}

const USMWorldVisualFragment* ASMBaseItemDropActor::FindWorldVisualFragment(
	const USMItemDefinition* InItemDefinition) const
{
	if (InItemDefinition == nullptr)
	{
		return nullptr;
	}

	return InItemDefinition->FindFragmentByClass<USMWorldVisualFragment>();
}
