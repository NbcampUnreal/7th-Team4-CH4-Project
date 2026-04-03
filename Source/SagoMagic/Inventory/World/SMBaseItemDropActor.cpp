#include "Inventory/World/SMBaseItemDropActor.h"

#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

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
}

void ASMBaseItemDropActor::BeginPlay()
{
	Super::BeginPlay();
}

void ASMBaseItemDropActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMBaseItemDropActor, ItemDropPayload);
}

void ASMBaseItemDropActor::InitializeFromPayload(const FSMItemDropPayload& InItemDropPayload)
{
	ItemDropPayload = InItemDropPayload;
	bInitialized = true;

	ApplyWorldVisual();
}

void ASMBaseItemDropActor::OnRep_ItemDropPayload()
{
	bInitialized = HasValidPayload();

	if (bInitialized)
	{
		ApplyWorldVisual();
	}
}

const USMItemDefinition* ASMBaseItemDropActor::ResolveItemDefinition() const
{
	return nullptr;
}

void ASMBaseItemDropActor::ApplyWorldVisual()
{
}

bool ASMBaseItemDropActor::HasValidPayload() const
{
	return false;
}

const USMWorldVisualFragment* ASMBaseItemDropActor::FindWorldVisualFragment(
	const USMItemDefinition* InItemDefinition) const
{
	return nullptr;
}
