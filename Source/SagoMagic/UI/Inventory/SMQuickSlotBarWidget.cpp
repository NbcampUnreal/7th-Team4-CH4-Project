#include "UI/Inventory/SMQuickSlotBarWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
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
#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

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

FReply USMQuickSlotBarWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InventoryComponent == nullptr)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	int32 SlotIndex = INDEX_NONE;
	FGuid SlotSkillInstanceId;
	if (FindSlotItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition(), SlotIndex, SlotSkillInstanceId) == false)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			OwningPanel->ShowHoveredItemInfo(SlotSkillInstanceId, InMouseEvent.GetScreenSpacePosition());
			OwningPanel->OpenContextMenuForItem(SlotSkillInstanceId, InMouseEvent.GetScreenSpacePosition());
			return FReply::Handled();
		}

		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OwningPanel->CloseContextMenu();
		}
	}

	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	PendingDragSlotIndex = SlotIndex;
	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

FReply USMQuickSlotBarWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply SuperReply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	if (InventoryComponent == nullptr)
	{
		return SuperReply;
	}

	int32 SlotIndex = INDEX_NONE;
	FGuid SlotSkillInstanceId;
	if (FindSlotItemAtScreenPosition(InMouseEvent.GetScreenSpacePosition(), SlotIndex, SlotSkillInstanceId))
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ShowHoveredItemInfo(SlotSkillInstanceId, InMouseEvent.GetScreenSpacePosition());
			return FReply::Handled();
		}
	}
	else if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->HideHoveredItemInfo();
	}

	return SuperReply;
}

void USMQuickSlotBarWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->HideHoveredItemInfo();
	}
}

void USMQuickSlotBarWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	OutOperation = nullptr;

	if (PendingDragSlotIndex == INDEX_NONE)
	{
		return;
	}

	OutOperation = CreateDragDropOperationForQuickSlotSkill(PendingDragSlotIndex, InGeometry, InMouseEvent);
	PendingDragSlotIndex = INDEX_NONE;
}

bool USMQuickSlotBarWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	USMInventoryDragDropOperation* InventoryOperation = Cast<USMInventoryDragDropOperation>(InOperation);
	if (InventoryOperation == nullptr || InventoryComponent == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}

	int32 TargetSlotIndex = INDEX_NONE;
	if (FindSlotIndexAtScreenPosition(InDragDropEvent.GetScreenSpacePosition(), TargetSlotIndex) == false)
	{
		return false;
	}

	if (InventoryOperation->GetSourceContainerId() != InventoryComponent->GetMainInventory().ContainerId)
	{
		return false;
	}

	const FSMQuickSlotEntry* TargetSlot = InventoryComponent->FindQuickSlotEntry(TargetSlotIndex);
	if (TargetSlot == nullptr)
	{
		return false;
	}

	return TargetSlot->GetEquippedSkillId().IsValid() == false;
}

bool USMQuickSlotBarWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	USMInventoryDragDropOperation* InventoryOperation = Cast<USMInventoryDragDropOperation>(InOperation);
	if (InventoryOperation == nullptr || InventoryComponent == nullptr)
	{
		return false;
	}

	int32 TargetSlotIndex = INDEX_NONE;
	if (FindSlotIndexAtScreenPosition(InDragDropEvent.GetScreenSpacePosition(), TargetSlotIndex) == false)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		return false;
	}

	if (InventoryOperation->GetSourceContainerId() != InventoryComponent->GetMainInventory().ContainerId)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		return false;
	}

	const FSMQuickSlotEntry* TargetSlot = InventoryComponent->FindQuickSlotEntry(TargetSlotIndex);
	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (TargetSlot == nullptr || TargetSlot->GetEquippedSkillId().IsValid() || OwningPlayerController == nullptr)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		return false;
	}

	OwningPlayerController->ServerRPCEquipSkillToQuickSlot(InventoryOperation->GetItemInstanceId(), TargetSlotIndex);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->ClearActiveDragState();
	}

	return true;
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
		FVector2D EffectivePreviewAreaSize = PreviewAreaSize;
		if (const FVector2D CanvasSize = TargetPreviewCanvas->GetCachedGeometry().GetLocalSize();
			CanvasSize.X > 0.0f && CanvasSize.Y > 0.0f)
		{
			EffectivePreviewAreaSize = CanvasSize;
		}

		const float WidthCellSize = EffectivePreviewAreaSize.X / static_cast<float>(PreviewWidth);
		const float HeightCellSize = EffectivePreviewAreaSize.Y / static_cast<float>(PreviewHeight);
		const float PreviewCellSize = FMath::Clamp(
			FMath::Min(WidthCellSize, HeightCellSize),
			MinPreviewCellSize,
			MaxPreviewCellSize);
		const FVector2D PreviewOffset(
			(EffectivePreviewAreaSize.X - (PreviewWidth * PreviewCellSize)) * 0.5f,
			(EffectivePreviewAreaSize.Y - (PreviewHeight * PreviewCellSize)) * 0.5f);

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

bool USMQuickSlotBarWidget::FindSlotIndexAtScreenPosition(const FVector2D& InScreenPosition, int32& OutSlotIndex) const
{
	OutSlotIndex = INDEX_NONE;

	auto IsUnderWidget = [&InScreenPosition](const UWidget* InWidget)
	{
		if (InWidget == nullptr)
		{
			return false;
		}

		const FGeometry& CachedGeometry = InWidget->GetCachedGeometry();
		const FVector2D LocalPosition = CachedGeometry.AbsoluteToLocal(InScreenPosition);
		const FVector2D LocalSize = CachedGeometry.GetLocalSize();
		return LocalPosition.X >= 0.0f &&
			LocalPosition.Y >= 0.0f &&
			LocalPosition.X <= LocalSize.X &&
			LocalPosition.Y <= LocalSize.Y;
	};

	if (IsUnderWidget(Slot1_BaseBackground))
	{
		OutSlotIndex = 0;
		return true;
	}

	if (IsUnderWidget(Slot2_BaseBackground))
	{
		OutSlotIndex = 1;
		return true;
	}

	return false;
}

bool USMQuickSlotBarWidget::FindSlotItemAtScreenPosition(
	const FVector2D& InScreenPosition,
	int32& OutSlotIndex,
	FGuid& OutSkillInstanceId) const
{
	OutSlotIndex = INDEX_NONE;
	OutSkillInstanceId.Invalidate();

	if (InventoryComponent == nullptr || FindSlotIndexAtScreenPosition(InScreenPosition, OutSlotIndex) == false)
	{
		return false;
	}

	const FSMQuickSlotEntry* SlotEntry = InventoryComponent->FindQuickSlotEntry(OutSlotIndex);
	if (SlotEntry == nullptr || SlotEntry->GetEquippedSkillId().IsValid() == false)
	{
		return false;
	}

	OutSkillInstanceId = SlotEntry->GetEquippedSkillId();
	return true;
}

UDragDropOperation* USMQuickSlotBarWidget::CreateDragDropOperationForQuickSlotSkill(
	int32 InSlotIndex,
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InventoryComponent == nullptr)
	{
		return nullptr;
	}

	const FSMQuickSlotEntry* SlotEntry = InventoryComponent->FindQuickSlotEntry(InSlotIndex);
	if (SlotEntry == nullptr || SlotEntry->GetEquippedSkillId().IsValid() == false)
	{
		return nullptr;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(SlotEntry->GetEquippedSkillId(), SkillData) == false)
	{
		return nullptr;
	}

	USMInventoryDragDropOperation* NewOperation = NewObject<USMInventoryDragDropOperation>(this);
	if (NewOperation == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = CreateWidget<USMDragItemPreviewWidget>(this, USMDragItemPreviewWidget::StaticClass());
	if (PreviewWidget != nullptr)
	{
		PreviewWidget->InitializePreviewFromInventory(SkillData.BaseItem.InstanceId, SkillData.BaseItem.Rotation, InventoryComponent);
	}

	int32 ShapeWidth = 1;
	int32 ShapeHeight = 1;
	if (const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(SkillData.BaseItem))
	{
		if (const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>())
		{
			const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
			if (ShapeMask.IsValidMaskData())
			{
				ShapeWidth = ShapeMask.Width;
				ShapeHeight = ShapeMask.Height;
			}
		}
	}

	NewOperation->InitializeOperation(
		SkillData.BaseItem.InstanceId,
		SlotEntry->GetContainerId(),
		0,
		0,
		SkillData.BaseItem.Rotation,
		0,
		0,
		ShapeWidth,
		ShapeHeight,
		FVector2D(0.5f, 0.5f),
		PreviewWidget);
	NewOperation->DefaultDragVisual = PreviewWidget;
	return NewOperation;
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
