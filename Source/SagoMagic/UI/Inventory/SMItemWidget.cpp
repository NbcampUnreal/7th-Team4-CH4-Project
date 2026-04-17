#include "UI/Inventory/SMItemWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMGridShapeFragment.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/Inventory/SMInventoryGridWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMItemWidget::USMItemWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , GridX(0)
	  , GridY(0)
	  , DisplayRotation(ESMGridRotation::Rot0)
	  , bDraggable(true)
	  , InventoryComponent(nullptr)
{
}

void USMItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply USMItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (ItemInstanceId.IsValid() == false || InventoryComponent == nullptr)
		{
			return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
		}

		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->SetHoveredItem(ItemInstanceId);
			OwningPanel->OpenContextMenuForItem(ItemInstanceId, InMouseEvent.GetScreenSpacePosition());
			return FReply::Handled();
		}

		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->CloseContextMenu();
		}
	}

	if (CanStartDrag() == false)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void USMItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (ItemInstanceId.IsValid() == false)
	{
		return;
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->SetHoveredItem(ItemInstanceId);
	}
}

void USMItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (OwningPanel->GetHoveredItemInstanceId() == ItemInstanceId)
		{
			OwningPanel->ClearHoveredItem();
		}
	}
}

void USMItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                         UDragDropOperation*& OutOperation)
{
	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		if (OwningPanel->GetHoveredItemInstanceId() == ItemInstanceId)
		{
			OwningPanel->ClearHoveredItem();
		}

		OwningPanel->CloseContextMenu();
	}

	OutOperation = CreateDragDropOperation(InGeometry, InMouseEvent);

	if (USMInventoryDragDropOperation* InventoryOperation = Cast<USMInventoryDragDropOperation>(OutOperation))
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->BeginActiveDragPreview(InventoryOperation, InMouseEvent.GetScreenSpacePosition());
		}
	}
}

void USMItemWidget::InitializeItemWidget(
	const FGuid& InItemInstanceId,
	const FGuid& InOwningContainerId,
	int32 InGridX,
	int32 InGridY,
	ESMGridRotation InDisplayRotation,
	USMInventoryComponent* InInventoryComponent)
{
	ItemInstanceId = InItemInstanceId;
	OwningContainerId = InOwningContainerId;
	GridX = InGridX;
	GridY = InGridY;
	DisplayRotation = InDisplayRotation;
	InventoryComponent = InInventoryComponent;

	RefreshItemWidget();
}

void USMItemWidget::RefreshItemWidget()
{
	UpdateDisplayFromInventory();
	BP_OnItemWidgetUpdated();
}

bool USMItemWidget::CanStartDrag() const
{
	return bDraggable && ItemInstanceId.IsValid() && InventoryComponent != nullptr;
}

USMInventoryDragDropOperation* USMItemWidget::CreateDragDropOperation(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (USMInventoryGridWidget* OwningGrid = GetTypedOuter<USMInventoryGridWidget>())
	{
		int32 ShapeWidth = 1;
		int32 ShapeHeight = 1;

		FSMItemInstanceData BaseItemData;
		if (InventoryComponent != nullptr)
		{
			FSMItemInstanceData ItemData;
			if (InventoryComponent->GetItemData(ItemInstanceId, ItemData))
			{
				BaseItemData = ItemData;
			}
			else
			{
				FSMSkillItemInstanceData SkillData;
				if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData))
				{
					BaseItemData = SkillData.BaseItem;
				}
			}

			if (BaseItemData.InstanceId.IsValid())
			{
				if (const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(BaseItemData))
				{
					if (const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>())
					{
						const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
						if (ShapeMask.IsValidMaskData())
						{
							ShapeWidth = FMath::Max(1, ShapeMask.Width);
							ShapeHeight = FMath::Max(1, ShapeMask.Height);
						}
					}
				}
			}
		}

		const bool bSwapDimensions = DisplayRotation == ESMGridRotation::Rot90 || DisplayRotation == ESMGridRotation::Rot270;
		const int32 CurrentWidth = bSwapDimensions ? ShapeHeight : ShapeWidth;
		const int32 CurrentHeight = bSwapDimensions ? ShapeWidth : ShapeHeight;

		int32 PivotGridX = GridX;
		int32 PivotGridY = GridY;
		FVector2D PivotCellFraction(0.5f, 0.5f);

		const FVector2D LocalMousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		const FVector2D LocalSize = InGeometry.GetLocalSize();
		if (CurrentWidth > 0 && CurrentHeight > 0 && LocalSize.X > 0.0f && LocalSize.Y > 0.0f)
		{
			const float NormalizedX = FMath::Clamp(LocalMousePosition.X / LocalSize.X, 0.0f, 0.9999f);
			const float NormalizedY = FMath::Clamp(LocalMousePosition.Y / LocalSize.Y, 0.0f, 0.9999f);
			const float ShapeSpaceX = NormalizedX * static_cast<float>(CurrentWidth);
			const float ShapeSpaceY = NormalizedY * static_cast<float>(CurrentHeight);

			const int32 PivotRotatedLocalX = FMath::Clamp(FMath::FloorToInt(ShapeSpaceX), 0, FMath::Max(0, CurrentWidth - 1));
			const int32 PivotRotatedLocalY = FMath::Clamp(FMath::FloorToInt(ShapeSpaceY), 0, FMath::Max(0, CurrentHeight - 1));

			PivotGridX = GridX + PivotRotatedLocalX;
			PivotGridY = GridY + PivotRotatedLocalY;
			PivotCellFraction.X = FMath::Clamp(ShapeSpaceX - static_cast<float>(PivotRotatedLocalX), 0.0f, 1.0f);
			PivotCellFraction.Y = FMath::Clamp(ShapeSpaceY - static_cast<float>(PivotRotatedLocalY), 0.0f, 1.0f);
		}

		return OwningGrid->CreateDragDropOperationForItem(ItemInstanceId, PivotGridX, PivotGridY, PivotCellFraction);
	}

	USMInventoryDragDropOperation* NewOperation = NewObject<USMInventoryDragDropOperation>(this);
	if (NewOperation == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = CreateDragPreviewWidget();

	NewOperation->InitializeOperation(
		ItemInstanceId,
		OwningContainerId,
		GridX,
		GridY,
		DisplayRotation,
		0,
		0,
		1,
		1,
		FVector2D(0.5f, 0.5f),
		PreviewWidget);

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		NewOperation->SetOwningInventoryPanel(OwningPanel);
	}

	return NewOperation;
}

USMDragItemPreviewWidget* USMItemWidget::CreateDragPreviewWidget()
{
	if (DragPreviewWidgetClass == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = CreateWidget<USMDragItemPreviewWidget>(this, DragPreviewWidgetClass);
	if (PreviewWidget == nullptr)
	{
		return nullptr;
	}

	PreviewWidget->InitializePreviewFromInventory(ItemInstanceId, DisplayRotation, InventoryComponent);
	return PreviewWidget;
}

void USMItemWidget::UpdateDisplayFromInventory()
{
	if (InventoryComponent == nullptr)
	{
		bDraggable = false;
		return;
	}

	FSMItemInstanceData ItemData;
	if (InventoryComponent->GetItemData(ItemInstanceId, ItemData))
	{
		OwningContainerId = ItemData.ParentContainerId;
		GridX = ItemData.GridX;
		GridY = ItemData.GridY;
		DisplayRotation = ItemData.Rotation;
		bDraggable = ItemData.bLocked == false;
		return;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent->GetSkillData(ItemInstanceId, SkillData))
	{
		OwningContainerId = SkillData.BaseItem.ParentContainerId;
		GridX = SkillData.BaseItem.GridX;
		GridY = SkillData.BaseItem.GridY;
		DisplayRotation = SkillData.BaseItem.Rotation;
		bDraggable = SkillData.BaseItem.bLocked == false;
		return;
	}

	bDraggable = false;
}
