#include "UI/Inventory/SMInventoryGridWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Blueprint/DragDropOperation.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMGridShapeFragment.h"

#include "UI/Inventory/SMInventoryCellWidget.h"
#include "UI/Inventory/SMItemWidget.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryGridWidget::USMInventoryGridWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , InventoryComponent(nullptr)
	  , CellLayerPanel(nullptr)
	  , ItemLayerPanel(nullptr)
	  , GridWidth(0)
	  , GridHeight(0)
	  , HoveredGridX(-1)
	  , HoveredGridY(-1)
	  , ActiveDragDropOperation(nullptr)
{
}

void USMInventoryGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

bool USMInventoryGridWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	USMInventoryDragDropOperation* InventoryOperation = GetInventoryDragDropOperation(InOperation);
	if (InventoryOperation == nullptr)
	{
		return Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);
	}

	ActiveDragDropOperation = InventoryOperation;

	int32 GridX = 0;
	int32 GridY = 0;
	if (CalculateDropGridPosition(InGeometry, InDragDropEvent, GridX, GridY) == false)
	{
		ClearHoveredCellState();
		UpdateDraggedPreviewState(InOperation, false);
		return true;
	}

	SetHoveredCell(GridX, GridY);

	bool bCanPlace = false;
	if (InventoryComponent != nullptr)
	{
		bCanPlace = InventoryComponent->CanPlaceItem(
			InventoryOperation->GetItemInstanceId(),
			ContainerId,
			GridX,
			GridY,
			InventoryOperation->GetCurrentRotation());
	}

	UpdateDraggedPreviewState(InOperation, bCanPlace);
	return true;
}

bool USMInventoryGridWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	USMInventoryDragDropOperation* InventoryOperation = GetInventoryDragDropOperation(InOperation);
	if (InventoryOperation == nullptr)
	{
		return false;
	}

	if (InventoryComponent == nullptr)
	{
		return false;
	}

	int32 GridX = 0;
	int32 GridY = 0;
	if (CalculateDropGridPosition(InGeometry, InDragDropEvent, GridX, GridY) == false)
	{
		return false;
	}

	const bool bMoveSucceeded = InventoryComponent->MoveItem(
		InventoryOperation->GetItemInstanceId(),
		ContainerId,
		GridX,
		GridY,
		InventoryOperation->GetCurrentRotation());

	ActiveDragDropOperation = nullptr;
	ClearHoveredCellState();

	if (bMoveSucceeded)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->RefreshPanel();
		}
		else
		{
			RefreshGrid();
		}
	}

	return bMoveSucceeded;
}

void USMInventoryGridWidget::InitializeGridWidget(const FGuid& InContainerId,
                                                  USMInventoryComponent* InInventoryComponent)
{
	ContainerId = InContainerId;
	InventoryComponent = InInventoryComponent;
	RefreshGrid();
}

void USMInventoryGridWidget::RefreshGrid()
{
	RefreshContainerSize();
	RebuildCellWidgets();
	RebuildItemWidgets();
	ClearHoveredCellState();
	BP_OnGridRefreshed();
}

void USMInventoryGridWidget::RequestRotateDraggedItem()
{
	if (ActiveDragDropOperation == nullptr)
	{
		return;
	}

	const ESMGridRotation CurrentRotation = ActiveDragDropOperation->GetCurrentRotation();
	ESMGridRotation NextRotation = ESMGridRotation::Rot0;

	switch (CurrentRotation)
	{
	case ESMGridRotation::Rot0:
		NextRotation = ESMGridRotation::Rot90;
		break;

	case ESMGridRotation::Rot90:
		NextRotation = ESMGridRotation::Rot180;
		break;

	case ESMGridRotation::Rot180:
		NextRotation = ESMGridRotation::Rot270;
		break;

	case ESMGridRotation::Rot270:
		NextRotation = ESMGridRotation::Rot0;
		break;

	default:
		NextRotation = ESMGridRotation::Rot0;
		break;
	}

	ActiveDragDropOperation->UpdateCurrentRotation(NextRotation);

	if (USMDragItemPreviewWidget* PreviewWidget = ActiveDragDropOperation->GetDragPreviewWidget())
	{
		PreviewWidget->UpdatePreviewRotation(NextRotation);
	}

	if (HoveredGridX < 0 || HoveredGridY < 0 || InventoryComponent == nullptr)
	{
		return;
	}

	const bool bCanPlace = InventoryComponent->CanPlaceItem(
		ActiveDragDropOperation->GetItemInstanceId(),
		ContainerId,
		HoveredGridX,
		HoveredGridY,
		NextRotation);

	UpdateDraggedPreviewState(ActiveDragDropOperation, bCanPlace);
	UpdateCellStates();
}

void USMInventoryGridWidget::ClearHoveredCellState()
{
	HoveredGridX = -1;
	HoveredGridY = -1;
	UpdateCellStates();
}

USMInventoryDragDropOperation* USMInventoryGridWidget::CreateDragDropOperationForItem(const FGuid& InItemInstanceId)
{
	if (InventoryComponent == nullptr || InItemInstanceId.IsValid() == false)
	{
		return nullptr;
	}

	FSMItemInstanceData BaseItemData;
	if (GetBaseItemData(InItemInstanceId, BaseItemData) == false)
	{
		return nullptr;
	}

	if (BaseItemData.bLocked)
	{
		return nullptr;
	}

	USMInventoryDragDropOperation* NewOperation = NewObject<USMInventoryDragDropOperation>(this);
	if (NewOperation == nullptr)
	{
		return nullptr;
	}

	USMDragItemPreviewWidget* PreviewWidget = nullptr;
	if (ItemWidgetClass != nullptr)
	{
		const USMItemWidget* ItemWidgetCDO = ItemWidgetClass->GetDefaultObject<USMItemWidget>();
		if (ItemWidgetCDO != nullptr && ItemWidgetCDO->GetDragPreviewWidgetClass() != nullptr)
		{
			PreviewWidget = CreateWidget<USMDragItemPreviewWidget>(this, ItemWidgetCDO->GetDragPreviewWidgetClass());
			if (PreviewWidget != nullptr)
			{
				PreviewWidget->InitializePreview(BaseItemData.InstanceId, BaseItemData.Rotation);
			}
		}
	}

	NewOperation->InitializeOperation(
		BaseItemData.InstanceId,
		BaseItemData.ParentContainerId,
		BaseItemData.GridX,
		BaseItemData.GridY,
		BaseItemData.Rotation,
		PreviewWidget);

	NewOperation->DefaultDragVisual = PreviewWidget;
	ActiveDragDropOperation = NewOperation;
	return NewOperation;
}

bool USMInventoryGridWidget::CalculateDropGridPosition(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	int32& OutGridX,
	int32& OutGridY) const
{
	OutGridX = -1;
	OutGridY = -1;

	if (GridWidth <= 0 || GridHeight <= 0)
	{
		return false;
	}

	const FVector2D LocalPosition = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FVector2D LocalSize = InGeometry.GetLocalSize();

	if (LocalSize.X <= 0.0f || LocalSize.Y <= 0.0f)
	{
		return false;
	}

	const float CellWidth = LocalSize.X / static_cast<float>(GridWidth);
	const float CellHeight = LocalSize.Y / static_cast<float>(GridHeight);

	if (CellWidth <= 0.0f || CellHeight <= 0.0f)
	{
		return false;
	}

	const int32 GridX = FMath::FloorToInt(LocalPosition.X / CellWidth);
	const int32 GridY = FMath::FloorToInt(LocalPosition.Y / CellHeight);

	if (GridX < 0 || GridX >= GridWidth || GridY < 0 || GridY >= GridHeight)
	{
		return false;
	}

	OutGridX = GridX;
	OutGridY = GridY;
	return true;
}

USMInventoryDragDropOperation* USMInventoryGridWidget::GetInventoryDragDropOperation(
	UDragDropOperation* InOperation) const
{
	return Cast<USMInventoryDragDropOperation>(InOperation);
}

void USMInventoryGridWidget::RebuildCellWidgets()
{
	ClearCellWidgets();

	if (CellLayerPanel == nullptr)
	{
		return;
	}

	if (CellWidgetClass == nullptr)
	{
		return;
	}

	if (InventoryComponent == nullptr)
	{
		return;
	}

	FSMGridContainerState ContainerData;
	if (InventoryComponent->GetContainerData(ContainerId, ContainerData) == false)
	{
		return;
	}

	const FString& BitMask = ContainerData.ValidMask.BitMask;
	const int32 MaskWidth = ContainerData.ValidMask.Width;
	const int32 MaskHeight = ContainerData.ValidMask.Height;

	for (int32 Y = 0; Y < MaskHeight; ++Y)
	{
		for (int32 X = 0; X < MaskWidth; ++X)
		{
			const int32 Index = (Y * MaskWidth) + X;
			const bool bCellEnabled =
				BitMask.IsValidIndex(Index) && BitMask[Index] == TEXT('1');

			USMInventoryCellWidget* NewCellWidget = CreateWidget<USMInventoryCellWidget>(this, CellWidgetClass);
			if (NewCellWidget == nullptr)
			{
				continue;
			}

			NewCellWidget->InitializeCellWidget(X, Y, bCellEnabled);
			CellLayerPanel->AddChild(NewCellWidget);
			ApplyCellWidgetLayout(NewCellWidget, X, Y);
			CellWidgets.Add(NewCellWidget);
		}
	}
}

void USMInventoryGridWidget::RebuildItemWidgets()
{
	ClearItemWidgets();

	if (ItemLayerPanel == nullptr)
	{
		return;
	}

	if (ItemWidgetClass == nullptr)
	{
		return;
	}

	if (InventoryComponent == nullptr)
	{
		return;
	}

	for (const FSMItemInstanceData& ItemData : InventoryComponent->GetItemEntries())
	{
		if (ItemData.ParentContainerId != ContainerId)
		{
			continue;
		}

		USMItemWidget* NewItemWidget = CreateWidget<USMItemWidget>(this, ItemWidgetClass);
		if (NewItemWidget == nullptr)
		{
			continue;
		}

		NewItemWidget->InitializeItemWidget(
			ItemData.InstanceId,
			ItemData.ParentContainerId,
			ItemData.GridX,
			ItemData.GridY,
			ItemData.Rotation,
			InventoryComponent);
		NewItemWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		ItemLayerPanel->AddChild(NewItemWidget);
		ApplyItemWidgetLayout(NewItemWidget, ItemData.GridX, ItemData.GridY);
		ApplyItemOwnershipToCells(ItemData);
	}

	for (const FSMSkillItemInstanceData& SkillData : InventoryComponent->GetSkillEntries())
	{
		if (SkillData.BaseItem.ParentContainerId != ContainerId)
		{
			continue;
		}

		USMItemWidget* NewItemWidget = CreateWidget<USMItemWidget>(this, ItemWidgetClass);
		if (NewItemWidget == nullptr)
		{
			continue;
		}

		NewItemWidget->InitializeItemWidget(
			SkillData.BaseItem.InstanceId,
			SkillData.BaseItem.ParentContainerId,
			SkillData.BaseItem.GridX,
			SkillData.BaseItem.GridY,
			SkillData.BaseItem.Rotation,
			InventoryComponent);
		NewItemWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		ItemLayerPanel->AddChild(NewItemWidget);
		ApplyItemWidgetLayout(NewItemWidget, SkillData.BaseItem.GridX, SkillData.BaseItem.GridY);
		ApplyItemOwnershipToCells(SkillData.BaseItem);
	}
}

void USMInventoryGridWidget::ClearCellWidgets()
{
	CellWidgets.Reset();

	if (CellLayerPanel == nullptr)
	{
		return;
	}

	CellLayerPanel->ClearChildren();
}

void USMInventoryGridWidget::ClearItemWidgets()
{
	if (ItemLayerPanel == nullptr)
	{
		return;
	}

	ItemLayerPanel->ClearChildren();
}

void USMInventoryGridWidget::UpdateCellStates()
{
	TArray<FIntPoint> HighlightedCells;
	bool bCanPlace = false;

	if (InventoryComponent != nullptr &&
		ActiveDragDropOperation != nullptr &&
		ActiveDragDropOperation->HasValidItemInstanceId() &&
		HoveredGridX >= 0 &&
		HoveredGridY >= 0)
	{
		bCanPlace = InventoryComponent->CanPlaceItem(
			ActiveDragDropOperation->GetItemInstanceId(),
			ContainerId,
			HoveredGridX,
			HoveredGridY,
			ActiveDragDropOperation->GetCurrentRotation());

		FSMItemInstanceData BaseItemData;
		const FSMItemInstanceData* ItemData = InventoryComponent->FindItem(
			ActiveDragDropOperation->GetItemInstanceId());
		const FSMSkillItemInstanceData* SkillData = ItemData == nullptr
			                                            ? InventoryComponent->FindSkill(
				                                            ActiveDragDropOperation->GetItemInstanceId())
			                                            : nullptr;

		if (ItemData != nullptr)
		{
			BaseItemData = *ItemData;
		}
		else if (SkillData != nullptr)
		{
			BaseItemData = SkillData->BaseItem;
		}

		if (BaseItemData.InstanceId.IsValid())
		{
			const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(BaseItemData);
			if (ItemDefinition != nullptr)
			{
				const USMGridShapeFragment* GridShapeFragment =
					ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
				if (GridShapeFragment != nullptr)
				{
					const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
					if (ShapeMask.IsValidMaskData())
					{
						for (int32 Y = 0; Y < ShapeMask.Height; ++Y)
						{
							for (int32 X = 0; X < ShapeMask.Width; ++X)
							{
								const int32 MaskIndex = (Y * ShapeMask.Width) + X;
								if (ShapeMask.BitMask.IsValidIndex(MaskIndex) == false ||
									ShapeMask.BitMask[MaskIndex] != TEXT('1'))
								{
									continue;
								}

								int32 RotatedX = X;
								int32 RotatedY = Y;

								switch (ActiveDragDropOperation->GetCurrentRotation())
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

								default:
									continue;
								}

								HighlightedCells.Add(FIntPoint(HoveredGridX + RotatedX, HoveredGridY + RotatedY));
							}
						}
					}
				}
			}
		}
	}

	for (USMInventoryCellWidget* CellWidget : CellWidgets)
	{
		if (CellWidget == nullptr)
		{
			continue;
		}

		const bool bHovered =
			CellWidget->GetGridX() == HoveredGridX &&
			CellWidget->GetGridY() == HoveredGridY;

		const bool bHighlighted =
			HighlightedCells.Contains(FIntPoint(CellWidget->GetGridX(), CellWidget->GetGridY()));
		const bool bPlaceableHighlighted = bHighlighted && bCanPlace;
		const bool bBlockedHighlighted = bHighlighted && bCanPlace == false;

		if (CellWidget->IsHoveredCell() == bHovered &&
			CellWidget->IsPlaceableHighlighted() == bPlaceableHighlighted &&
			CellWidget->IsBlockedHighlighted() == bBlockedHighlighted)
		{
			continue;
		}

		CellWidget->UpdateCellState(
			CellWidget->IsCellEnabled(),
			bHovered,
			bPlaceableHighlighted,
			bBlockedHighlighted);
	}
}

void USMInventoryGridWidget::UpdateDraggedPreviewState(UDragDropOperation* InOperation, bool bInCanPlace)
{
	USMInventoryDragDropOperation* InventoryOperation = GetInventoryDragDropOperation(InOperation);
	if (InventoryOperation == nullptr)
	{
		return;
	}

	USMDragItemPreviewWidget* PreviewWidget = InventoryOperation->GetDragPreviewWidget();
	if (PreviewWidget == nullptr)
	{
		return;
	}

	PreviewWidget->UpdatePlaceableState(bInCanPlace);
}

USMInventoryCellWidget* USMInventoryGridWidget::FindCellWidget(int32 InGridX, int32 InGridY) const
{
	for (USMInventoryCellWidget* CellWidget : CellWidgets)
	{
		if (CellWidget == nullptr)
		{
			continue;
		}

		if (CellWidget->GetGridX() == InGridX && CellWidget->GetGridY() == InGridY)
		{
			return CellWidget;
		}
	}

	return nullptr;
}

void USMInventoryGridWidget::SetHoveredCell(int32 InGridX, int32 InGridY)
{
	HoveredGridX = InGridX;
	HoveredGridY = InGridY;
	UpdateCellStates();
}

bool USMInventoryGridWidget::GetBaseItemData(const FGuid& InItemInstanceId, FSMItemInstanceData& OutBaseItemData) const
{
	FSMItemInstanceData ItemData;
	if (InventoryComponent != nullptr && InventoryComponent->GetItemData(InItemInstanceId, ItemData))
	{
		OutBaseItemData = ItemData;
		return true;
	}

	FSMSkillItemInstanceData SkillData;
	if (InventoryComponent != nullptr && InventoryComponent->GetSkillData(InItemInstanceId, SkillData))
	{
		OutBaseItemData = SkillData.BaseItem;
		return true;
	}

	return false;
}

bool USMInventoryGridWidget::BuildOccupiedCellsFromItemData(const FSMItemInstanceData& InBaseItemData,
                                                            TArray<FIntPoint>& OutOccupiedCells) const
{
	OutOccupiedCells.Reset();

	if (InventoryComponent == nullptr || InBaseItemData.InstanceId.IsValid() == false)
	{
		return false;
	}

	const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(InBaseItemData);
	if (ItemDefinition == nullptr)
	{
		OutOccupiedCells.Add(FIntPoint(InBaseItemData.GridX, InBaseItemData.GridY));
		return true;
	}

	const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
	if (GridShapeFragment == nullptr)
	{
		OutOccupiedCells.Add(FIntPoint(InBaseItemData.GridX, InBaseItemData.GridY));
		return true;
	}

	const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
	if (ShapeMask.IsValidMaskData() == false)
	{
		OutOccupiedCells.Add(FIntPoint(InBaseItemData.GridX, InBaseItemData.GridY));
		return true;
	}

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

			switch (InBaseItemData.Rotation)
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

			default:
				continue;
			}

			OutOccupiedCells.Add(FIntPoint(InBaseItemData.GridX + RotatedX, InBaseItemData.GridY + RotatedY));
		}
	}

	return true;
}

void USMInventoryGridWidget::ApplyItemOwnershipToCells(const FSMItemInstanceData& InBaseItemData)
{
	TArray<FIntPoint> OccupiedCells;
	if (BuildOccupiedCellsFromItemData(InBaseItemData, OccupiedCells) == false)
	{
		return;
	}

	for (const FIntPoint& OccupiedCell : OccupiedCells)
	{
		USMInventoryCellWidget* CellWidget = FindCellWidget(OccupiedCell.X, OccupiedCell.Y);
		if (CellWidget == nullptr)
		{
			continue;
		}

		CellWidget->UpdateOccupiedItem(InBaseItemData.InstanceId);
	}
}

void USMInventoryGridWidget::RefreshContainerSize()
{
	GridWidth = 0;
	GridHeight = 0;

	if (InventoryComponent == nullptr)
	{
		return;
	}

	FSMGridContainerState ContainerData;
	if (InventoryComponent->GetContainerData(ContainerId, ContainerData) == false)
	{
		return;
	}

	GridWidth = ContainerData.ValidMask.Width;
	GridHeight = ContainerData.ValidMask.Height;
}

void USMInventoryGridWidget::ApplyCellWidgetLayout(USMInventoryCellWidget* InCellWidget, int32 InGridX, int32 InGridY)
{
	if (InCellWidget == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InCellWidget->Slot);
	if (CanvasSlot == nullptr)
	{
		return;
	}

	if (GridWidth <= 0 || GridHeight <= 0)
	{
		return;
	}

	const FVector2D MinAnchor(
		static_cast<float>(InGridX) / static_cast<float>(GridWidth),
		static_cast<float>(InGridY) / static_cast<float>(GridHeight));
	const FVector2D MaxAnchor(
		static_cast<float>(InGridX + 1) / static_cast<float>(GridWidth),
		static_cast<float>(InGridY + 1) / static_cast<float>(GridHeight));

	CanvasSlot->SetAnchors(FAnchors(MinAnchor.X, MinAnchor.Y, MaxAnchor.X, MaxAnchor.Y));
	CanvasSlot->SetOffsets(FMargin(0.0f));
	CanvasSlot->SetAlignment(FVector2D::ZeroVector);
	CanvasSlot->SetAutoSize(false);
	CanvasSlot->SetZOrder(0);
}

void USMInventoryGridWidget::ApplyItemWidgetLayout(USMItemWidget* InItemWidget, int32 InGridX, int32 InGridY)
{
	if (InItemWidget == nullptr)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(InItemWidget->Slot);
	if (CanvasSlot == nullptr)
	{
		return;
	}

	if (GridWidth <= 0 || GridHeight <= 0)
	{
		return;
	}

	int32 MinGridX = InGridX;
	int32 MinGridY = InGridY;
	int32 MaxGridX = InGridX;
	int32 MaxGridY = InGridY;

	if (InventoryComponent != nullptr)
	{
		FSMItemInstanceData BaseItemData;
		const FSMItemInstanceData* ItemData = InventoryComponent->FindItem(InItemWidget->GetItemInstanceId());
		const FSMSkillItemInstanceData* SkillData = ItemData == nullptr
			                                            ? InventoryComponent->FindSkill(
				                                            InItemWidget->GetItemInstanceId())
			                                            : nullptr;

		if (ItemData != nullptr)
		{
			BaseItemData = *ItemData;
		}
		else if (SkillData != nullptr)
		{
			BaseItemData = SkillData->BaseItem;
		}

		if (BaseItemData.InstanceId.IsValid())
		{
			const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(BaseItemData);
			if (ItemDefinition != nullptr)
			{
				const USMGridShapeFragment* GridShapeFragment =
					ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
				if (GridShapeFragment != nullptr)
				{
					const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
					if (ShapeMask.IsValidMaskData())
					{
						bool bInitializedBounds = false;

						for (int32 Y = 0; Y < ShapeMask.Height; ++Y)
						{
							for (int32 X = 0; X < ShapeMask.Width; ++X)
							{
								const int32 MaskIndex = (Y * ShapeMask.Width) + X;
								if (ShapeMask.BitMask.IsValidIndex(MaskIndex) == false ||
									ShapeMask.BitMask[MaskIndex] != TEXT('1'))
								{
									continue;
								}

								int32 RotatedX = X;
								int32 RotatedY = Y;

								switch (InItemWidget->GetDisplayRotation())
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

								default:
									continue;
								}

								const int32 CellX = InGridX + RotatedX;
								const int32 CellY = InGridY + RotatedY;

								if (bInitializedBounds == false)
								{
									MinGridX = CellX;
									MinGridY = CellY;
									MaxGridX = CellX;
									MaxGridY = CellY;
									bInitializedBounds = true;
								}
								else
								{
									MinGridX = FMath::Min(MinGridX, CellX);
									MinGridY = FMath::Min(MinGridY, CellY);
									MaxGridX = FMath::Max(MaxGridX, CellX);
									MaxGridY = FMath::Max(MaxGridY, CellY);
								}
							}
						}
					}
				}
			}
		}
	}

	const FVector2D MinAnchor(
		static_cast<float>(FMath::Clamp(MinGridX, 0, GridWidth)) / static_cast<float>(GridWidth),
		static_cast<float>(FMath::Clamp(MinGridY, 0, GridHeight)) / static_cast<float>(GridHeight));
	const FVector2D MaxAnchor(
		static_cast<float>(FMath::Clamp(MaxGridX + 1, 0, GridWidth)) / static_cast<float>(GridWidth),
		static_cast<float>(FMath::Clamp(MaxGridY + 1, 0, GridHeight)) / static_cast<float>(GridHeight));

	CanvasSlot->SetAnchors(FAnchors(MinAnchor.X, MinAnchor.Y, MaxAnchor.X, MaxAnchor.Y));
	CanvasSlot->SetOffsets(FMargin(0.0f));
	CanvasSlot->SetAlignment(FVector2D::ZeroVector);
	CanvasSlot->SetAutoSize(false);
	CanvasSlot->SetZOrder(1);
}
