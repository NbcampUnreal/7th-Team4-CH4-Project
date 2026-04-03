#include "Inventory/World/SMBaseItemDropActor.h"

#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

#include "Components/SMInteractionTargetComponent.h"

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

	InteractionTargetComponent = CreateDefaultSubobject<USMInteractionTargetComponent>(TEXT("InteractionTargetComponent"));

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

	ApplyWorldVisual();
	RefreshInteractionState();
}

void ASMBaseItemDropActor::HandleInteract(APawn* InInteractingPawn)
{
	if (InInteractingPawn == nullptr)
	{
		return;
	}

	if (HasValidPayload() == false)
	{
		return;
	}

	/**
	 * TODO:
	 * 여기에서 InInteractingPawn 기준으로
	 * PlayerController / PlayerState / InventoryComponent를 찾아
	 * AddItemFromDropPayload(ItemDropPayload)를 호출합니다.
	 *
	 * 성공 시 이 액터를 Destroy() 처리합니다.
	 */
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

const USMWorldVisualFragment* ASMBaseItemDropActor::FindWorldVisualFragment(const USMItemDefinition* InItemDefinition) const
{
	if (InItemDefinition == nullptr)
	{
		return nullptr;
	}

	return InItemDefinition->FindFragmentByClass<USMWorldVisualFragment>();
}