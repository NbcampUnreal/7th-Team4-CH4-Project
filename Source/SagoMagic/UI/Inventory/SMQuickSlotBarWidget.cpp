#include "UI/Inventory/SMQuickSlotBarWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Character/SMPlayerController.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "GameplayTags/Message/SMMessageTag.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"
#include "Inventory/Items/Fragments/SMGridShapeFragment.h"

USMQuickSlotBarWidget::USMQuickSlotBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , ActiveSlotIndex(0)
{
}

void USMQuickSlotBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USMQuickSlotBarWidget::NativeDestruct()
{
	UnregisterQuickSlotMessageListener();
	Super::NativeDestruct();
}

void USMQuickSlotBarWidget::InitializeQuickSlotBarWidget(USMInventoryComponent* InInventoryComponent)
{
	UnregisterQuickSlotMessageListener();
	InventoryComponent = InInventoryComponent;
	RegisterQuickSlotMessageListener();
	RefreshQuickSlotBar();
}

void USMQuickSlotBarWidget::RefreshQuickSlotBar()
{
	SyncFromInventoryComponent();
	RebuildSlotPreviewVisuals();
	BP_OnQuickSlotBarUpdated();
}

void USMQuickSlotBarWidget::ActivateFirstSlot()
{
	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCSetActiveQuickSlot(0);
}

void USMQuickSlotBarWidget::ActivateSecondSlot()
{
	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	OwningPlayerController->ServerRPCSetActiveQuickSlot(1);
}

void USMQuickSlotBarWidget::SyncFromInventoryComponent()
{
	if (InventoryComponent == nullptr)
	{
		Slot1SkillId.Invalidate();
		Slot2SkillId.Invalidate();
		Slots.Reset();
		ActiveSlotIndex = 0;
		return;
	}

	const FSMQuickSlotSetState& QuickSlots = InventoryComponent->GetQuickSlots();
	Slots = QuickSlots.Slots;
	Slot1SkillId = QuickSlots.Slot1SkillId;
	Slot2SkillId = QuickSlots.Slot2SkillId;
	ActiveSlotIndex = QuickSlots.ActiveSlotIndex;

	if (Slot1SkillId.IsValid() == false)
	{
		for (const FSMQuickSlotEntry& SlotEntry : Slots)
		{
			if (SlotEntry.GetSlotIndex() == 0)
			{
				Slot1SkillId = SlotEntry.GetEquippedSkillId();
				break;
			}
		}
	}

	if (Slot2SkillId.IsValid() == false)
	{
		for (const FSMQuickSlotEntry& SlotEntry : Slots)
		{
			if (SlotEntry.GetSlotIndex() == 1)
			{
				Slot2SkillId = SlotEntry.GetEquippedSkillId();
				break;
			}
		}
	}
}

void USMQuickSlotBarWidget::RebuildSlotPreviewVisuals()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	auto ClearPreviewCanvas = [](UCanvasPanel* InCanvasPanel)
	{
		if (InCanvasPanel != nullptr)
		{
			InCanvasPanel->ClearChildren();
		}
	};

	ClearPreviewCanvas(Slot1_PreviewCanvas);
	ClearPreviewCanvas(Slot2_PreviewCanvas);

	if (Slot1_ActiveOutline != nullptr)
	{
		Slot1_ActiveOutline->SetVisibility(ActiveSlotIndex == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (Slot2_ActiveOutline != nullptr)
	{
		Slot2_ActiveOutline->SetVisibility(ActiveSlotIndex == 1 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (InventoryComponent == nullptr)
	{
		return;
	}

	for (const FSMQuickSlotEntry& SlotEntry : Slots)
	{
		UCanvasPanel* TargetPreviewCanvas = nullptr;
		if (SlotEntry.GetSlotIndex() == 0)
		{
			TargetPreviewCanvas = Slot1_PreviewCanvas;
		}
		else if (SlotEntry.GetSlotIndex() == 1)
		{
			TargetPreviewCanvas = Slot2_PreviewCanvas;
		}

		if (TargetPreviewCanvas == nullptr || SlotEntry.GetEquippedSkillId().IsValid() == false)
		{
			continue;
		}

		FSMSkillItemInstanceData SkillData;
		if (InventoryComponent->GetSkillData(SlotEntry.GetEquippedSkillId(), SkillData) == false)
		{
			continue;
		}

		const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(SkillData.BaseItem);
		if (ItemDefinition == nullptr)
		{
			continue;
		}

		FLinearColor PreviewAccentColor = FLinearColor::White;
		if (const USMDisplayInfoFragment* DisplayInfoFragment = ItemDefinition->FindFragmentByClass<USMDisplayInfoFragment>())
		{
			PreviewAccentColor = DisplayInfoFragment->GetAccentColor();
		}

		TArray<FIntPoint> OccupiedCells;
		if (const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>())
		{
			const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
			if (ShapeMask.IsValidMaskData())
			{
				for (int32 Y = 0; Y < ShapeMask.Height; ++Y)
				{
					for (int32 X = 0; X < ShapeMask.Width; ++X)
					{
						const int32 MaskIndex = (Y * ShapeMask.Width) + X;
						if (ShapeMask.BitMask.IsValidIndex(MaskIndex) == false || ShapeMask.BitMask[MaskIndex] != TEXT('1'))
						{
							continue;
						}

						int32 RotatedX = X;
						int32 RotatedY = Y;
						switch (SkillData.BaseItem.Rotation)
						{
						case ESMGridRotation::Rot0:
							break;

						case ESMGridRotation::Rot90:
							RotatedX = ShapeMask.Height - 1 - Y;
							RotatedY = X;
							break;

						case ESMGridRotation::Rot180:
							RotatedX = ShapeMask.Width - 1 - X;
							RotatedY = ShapeMask.Height - 1 - Y;
							break;

						case ESMGridRotation::Rot270:
							RotatedX = Y;
							RotatedY = ShapeMask.Width - 1 - X;
							break;
						}

						OccupiedCells.Add(FIntPoint(RotatedX, RotatedY));
					}
				}
			}
		}

		if (OccupiedCells.IsEmpty())
		{
			OccupiedCells.Add(FIntPoint::ZeroValue);
		}

		int32 MinX = OccupiedCells[0].X;
		int32 MinY = OccupiedCells[0].Y;
		int32 MaxX = OccupiedCells[0].X;
		int32 MaxY = OccupiedCells[0].Y;
		for (const FIntPoint& OccupiedCell : OccupiedCells)
		{
			MinX = FMath::Min(MinX, OccupiedCell.X);
			MinY = FMath::Min(MinY, OccupiedCell.Y);
			MaxX = FMath::Max(MaxX, OccupiedCell.X);
			MaxY = FMath::Max(MaxY, OccupiedCell.Y);
		}

		const int32 PreviewWidth = FMath::Max(1, MaxX - MinX + 1);
		const int32 PreviewHeight = FMath::Max(1, MaxY - MinY + 1);
		const float WidthCellSize = PreviewAreaSize.X / static_cast<float>(PreviewWidth);
		const float HeightCellSize = PreviewAreaSize.Y / static_cast<float>(PreviewHeight);
		const float PreviewCellSize = FMath::Clamp(
			FMath::Min(WidthCellSize, HeightCellSize),
			MinPreviewCellSize,
			MaxPreviewCellSize);
		const FVector2D PreviewOffset(
			(PreviewAreaSize.X - (PreviewWidth * PreviewCellSize)) * 0.5f,
			(PreviewAreaSize.Y - (PreviewHeight * PreviewCellSize)) * 0.5f);

		for (const FIntPoint& OccupiedCell : OccupiedCells)
		{
			USizeBox* PreviewCellSizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			if (PreviewCellSizeBox == nullptr)
			{
				continue;
			}

			PreviewCellSizeBox->SetWidthOverride(PreviewCellSize);
			PreviewCellSizeBox->SetHeightOverride(PreviewCellSize);

			UBorder* PreviewCellBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
			if (PreviewCellBorder == nullptr)
			{
				continue;
			}

			PreviewCellBorder->SetBrushColor(PreviewAccentColor);
			PreviewCellSizeBox->SetContent(PreviewCellBorder);

			if (UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(TargetPreviewCanvas->AddChild(PreviewCellSizeBox)))
			{
				PreviewCanvasSlot->SetAutoSize(false);
				PreviewCanvasSlot->SetSize(FVector2D(PreviewCellSize, PreviewCellSize));
				PreviewCanvasSlot->SetPosition(FVector2D(
					PreviewOffset.X + ((OccupiedCell.X - MinX) * PreviewCellSize),
					PreviewOffset.Y + ((OccupiedCell.Y - MinY) * PreviewCellSize)));
			}
		}
	}
}

void USMQuickSlotBarWidget::RegisterQuickSlotMessageListener()
{
	if (InventoryComponent == nullptr)
	{
		return;
	}

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	QuickSlotUpdatedListenerHandle = MessageSubsystem.RegisterListener<FSMQuickSlotUpdatedMessage>(
		SMMessageTag::Inventory_QuickSlotUpdated,
		this,
		&ThisClass::HandleQuickSlotUpdatedMessage);
}

void USMQuickSlotBarWidget::UnregisterQuickSlotMessageListener()
{
	if (QuickSlotUpdatedListenerHandle.IsValid())
	{
		QuickSlotUpdatedListenerHandle.Unregister();
	}
}

void USMQuickSlotBarWidget::HandleQuickSlotUpdatedMessage(
	FGameplayTag InChannel,
	const FSMQuickSlotUpdatedMessage& InMessage)
{
	APlayerController* OwningPlayerController = GetOwningPlayer();
	if (OwningPlayerController == nullptr)
	{
		return;
	}

	APlayerState* OwningPlayerState = OwningPlayerController->GetPlayerState<APlayerState>();
	if (OwningPlayerState == nullptr || InMessage.GetOwningPlayerState() != OwningPlayerState)
	{
		return;
	}

	RefreshQuickSlotBar();
}
