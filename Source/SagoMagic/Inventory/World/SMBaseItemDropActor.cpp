#include "Inventory/World/SMBaseItemDropActor.h"

#include "Net/UnrealNetwork.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Character/SMPlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#include "Components/SMInteractionTargetComponent.h"
#include "Components/SMInteractionWorldWidgetComponent.h"
#include "Inventory/Components/SMInventoryComponent.h"

#include "Inventory/Items/Definitions/SMGemItemDefinition.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Definitions/SMSkillItemDefinition.h"
#include "Inventory/Items/Fragments/SMGemModifierFragment.h"
#include "Inventory/Items/Fragments/SMGridShapeFragment.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"
#include "Inventory/Items/Fragments/SMSkillProgressionFragment.h"
#include "Inventory/Items/Fragments/SMWorldVisualFragment.h"
#include "GameplayTags/UI/SMUITag.h"
#include "UI/Inventory/SMInteractionWorldInfoWidget.h"

ASMBaseItemDropActor::ASMBaseItemDropActor()
	: bInitialized(false)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootSceneComponent);

	InteractionTargetComponent = CreateDefaultSubobject<USMInteractionTargetComponent>(
		TEXT("InteractionTargetComponent"));

	InteractionWorldWidgetComponent = CreateDefaultSubobject<USMInteractionWorldWidgetComponent>(
		TEXT("InteractionWorldWidgetComponent"));
	InteractionWorldWidgetComponent->SetupAttachment(RootSceneComponent);
	InteractionWorldWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionWorldWidgetComponent->SetGenerateOverlapEvents(false);

	if (InteractionTargetComponent != nullptr)
	{
		InteractionTargetComponent->SetInteractionDisplayText(FText::FromString(TEXT("습득")));
		InteractionTargetComponent->SetInteractionEnabled(false);
	}
}

void ASMBaseItemDropActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionWorldWidgetComponent != nullptr)
	{
		InteractionWorldWidgetComponent->SetRelativeLocation(InteractionInfoWidgetOffset);
	}

	RefreshInteractionState();
}

void ASMBaseItemDropActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetNetMode() == NM_DedicatedServer || StaticMeshComponent == nullptr || HasValidPayload() == false)
	{
		return;
	}

	StaticMeshComponent->AddLocalRotation(FRotator(0.0f, RotationSpeedDegreesPerSecond * DeltaTime, 0.0f));
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

	FText FailureMessage;
	const FGuid AddedItemInstanceId = InventoryComponent->AddItemFromDropPayloadWithFailureMessage(ItemDropPayload, FailureMessage);
	if (AddedItemInstanceId.IsValid() == false)
	{
		if (FailureMessage.IsEmpty() == false)
		{
			if (ASMPlayerController* PlayerController = Cast<ASMPlayerController>(InInteractingPawn->GetController()))
			{
				PlayerController->ClientRPC_ShowNotification(SMUITag::Event_Notification, FailureMessage, 2.0f);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Guid for current item is invalid. can't get item from actor %s"),
		       *InventoryComponent->GetName());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully added item from drop payload to inventory. ItemInstanceId: %s"),
	       *AddedItemInstanceId.ToString());
	Destroy();
}

void ASMBaseItemDropActor::ShowInteractionWorldInfo()
{
	if (InteractionWorldWidgetComponent == nullptr)
	{
		return;
	}

	FSMInteractionWorldInfoData DisplayData;
	if (BuildInteractionWorldInfoData(DisplayData) == false)
	{
		InteractionWorldWidgetComponent->HideInteractionInfo();
		return;
	}

	InteractionWorldWidgetComponent->ShowInteractionInfo(DisplayData);
}

void ASMBaseItemDropActor::HideInteractionWorldInfo()
{
	if (InteractionWorldWidgetComponent == nullptr)
	{
		return;
	}

	InteractionWorldWidgetComponent->HideInteractionInfo();
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

bool ASMBaseItemDropActor::BuildInteractionWorldInfoData(FSMInteractionWorldInfoData& OutDisplayData) const
{
	OutDisplayData = FSMInteractionWorldInfoData();

	const USMItemDefinition* ItemDefinition = ResolveItemDefinition();
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	if (const USMDisplayInfoFragment* DisplayInfoFragment =
		ItemDefinition->FindFragmentByClass<USMDisplayInfoFragment>())
	{
		OutDisplayData.DisplayName = DisplayInfoFragment->GetDisplayName();
		OutDisplayData.Description = DisplayInfoFragment->GetDescription();
		OutDisplayData.AccentColor = DisplayInfoFragment->GetAccentColor();
	}

	if (const USMGridShapeFragment* GridShapeFragment =
		ItemDefinition->FindFragmentByClass<USMGridShapeFragment>())
	{
		OutDisplayData.ShapeMask = GridShapeFragment->GetShapeMask();
	}

	const bool bIsSkillItem =
		ItemDropPayload.ItemType == ESMItemType::Skill || ItemDefinition->IsA<USMSkillItemDefinition>();
	const bool bIsGemItem =
		ItemDropPayload.ItemType == ESMItemType::Gem || ItemDefinition->IsA<USMGemItemDefinition>();

	bool bHasEmbeddedGem = false;
	for (const FSMNestedItemDropSnapshot& Snapshot : ItemDropPayload.GetNestedItemSnapshots())
	{
		if (Snapshot.ItemType == ESMItemType::Gem)
		{
			bHasEmbeddedGem = true;
			break;
		}
	}

	if (bIsGemItem)
	{
		int32 ModifierValue = 0;
		if (const USMGemModifierFragment* GemModifierFragment =
			ItemDefinition->FindFragmentByClass<USMGemModifierFragment>())
		{
			ModifierValue = GemModifierFragment->GetModifierValue();
		}

		FString SummaryString = FString::Printf(TEXT("%d%%"), ModifierValue);

		OutDisplayData.SummaryText = FText::FromString(SummaryString);
	}
	else if (bIsSkillItem)
	{
		int32 CurrentLevel = 1;
		int32 MaxLevel = TNumericLimits<int32>::Max();
		bool bLevelFromEmbeddedSameSkill = true;

		if (const USMSkillProgressionFragment* SkillProgressionFragment =
			ItemDefinition->FindFragmentByClass<USMSkillProgressionFragment>())
		{
			CurrentLevel = FMath::Max(1, SkillProgressionFragment->GetBaseLevel());
			MaxLevel = FMath::Max(CurrentLevel, SkillProgressionFragment->GetMaxLevel());
			bLevelFromEmbeddedSameSkill = SkillProgressionFragment->IsLevelFromEmbeddedSameSkill();
		}

		if (bLevelFromEmbeddedSameSkill)
		{
			for (const FSMNestedItemDropSnapshot& Snapshot : ItemDropPayload.GetNestedItemSnapshots())
			{
				if (Snapshot.GetParentSkillInstanceId() != ItemDropPayload.GetInstanceId())
				{
					continue;
				}

				if (Snapshot.ItemType != ESMItemType::Skill)
				{
					continue;
				}

				if (Snapshot.GetDefinition().ToSoftObjectPath() != ItemDropPayload.GetDefinition().ToSoftObjectPath())
				{
					continue;
				}

				++CurrentLevel;
			}
		}

		CurrentLevel = FMath::Clamp(CurrentLevel, 1, MaxLevel);

		FString SummaryString = FString::Printf(TEXT("레벨: %d"), CurrentLevel);
		if (bHasEmbeddedGem)
		{
			SummaryString += TEXT(" | 젬 장착중");
		}

		OutDisplayData.SummaryText = FText::FromString(SummaryString);
	}
	else if (InteractionTargetComponent != nullptr)
	{
		OutDisplayData.SummaryText = InteractionTargetComponent->GetInteractionDisplayText();
	}

	return true;
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
