#include "Inventory/World/SMBaseItemDropActor.h"

#include "Net/UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Components/SMInteractionTargetComponent.h"
#include "Inventory/Components/SMInventoryComponent.h"

#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"
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

	USMInventoryComponent* InventoryComponent = InteractingPlayerState->FindComponentByClass<USMInventoryComponent>();
	if (InventoryComponent == nullptr)
	{
		return;
	}

	const FGuid AddedItemInstanceId = InventoryComponent->AddItemFromDropPayload(ItemDropPayload);
	if (AddedItemInstanceId.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Guid for current item is invalid. can't get item from actor %s"),
		       *InventoryComponent->GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully added item from drop payload to inventory. ItemInstanceId: %s"),
	       *AddedItemInstanceId.ToString());
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
	if (ItemDropPayload.GetDefinition().IsNull())
	{
		return nullptr;
	}

	return ItemDropPayload.GetDefinition().LoadSynchronous();
}

void ASMBaseItemDropActor::ApplyWorldVisual()
{
	static const FName ColorMaterialSlotName(TEXT("Color"));
	static const FName BaseColorParameterName(TEXT("BaseColor"));

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

	if (WorldVisualFragment->GetWorldMesh().IsNull() == false)
	{
		if (UStaticMesh* WorldMesh = WorldVisualFragment->GetWorldMesh().LoadSynchronous())
		{
			StaticMeshComponent->SetStaticMesh(WorldMesh);
		}
	}

	StaticMeshComponent->SetWorldScale3D(WorldVisualFragment->GetWorldScale());

	if (WorldVisualFragment->GetColorSlotMaterial().IsNull() == false)
	{
		const int32 ColorMaterialSlotIndex = StaticMeshComponent->GetMaterialIndex(ColorMaterialSlotName);
		if (ColorMaterialSlotIndex != INDEX_NONE)
		{
			FLinearColor AccentColor = FLinearColor::White;
			if (const USMDisplayInfoFragment* DisplayInfoFragment =
				ItemDefinition->FindFragmentByClass<USMDisplayInfoFragment>())
			{
				AccentColor = DisplayInfoFragment->GetAccentColor();
			}

			UMaterialInstanceDynamic* ColorMaterialInstance =
				Cast<UMaterialInstanceDynamic>(StaticMeshComponent->GetMaterial(ColorMaterialSlotIndex));
			if (ColorMaterialInstance == nullptr)
			{
				if (UMaterialInterface* ColorSlotMaterial = WorldVisualFragment->GetColorSlotMaterial().LoadSynchronous())
				{
					ColorMaterialInstance =
						StaticMeshComponent->CreateDynamicMaterialInstance(ColorMaterialSlotIndex, ColorSlotMaterial);
					if (ColorMaterialInstance == nullptr)
					{
						StaticMeshComponent->SetMaterial(ColorMaterialSlotIndex, ColorSlotMaterial);
					}
				}
			}

			if (ColorMaterialInstance != nullptr)
			{
				ColorMaterialInstance->SetVectorParameterValue(BaseColorParameterName, AccentColor);
			}
		}
	}

	if (InteractionTargetComponent != nullptr)
	{
		UMaterialInterface* HighlightOverlayMaterial = nullptr;

		if (WorldVisualFragment->GetOverrideMaterial().IsNull() == false)
		{
			HighlightOverlayMaterial = WorldVisualFragment->GetOverrideMaterial().LoadSynchronous();
		}

		InteractionTargetComponent->SetHighlightOverlayMaterial(HighlightOverlayMaterial);
	}
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
