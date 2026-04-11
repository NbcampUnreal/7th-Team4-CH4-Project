#include "UI/Inventory/SMInventoryGridWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Blueprint/DragDropOperation.h"

#include "Character/SMPlayerController.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemInstanceTypes.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "Inventory/Items/Fragments/SMDisplayInfoFragment.h"
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
	  , CachedStructureWidth(0)
	  , CachedStructureHeight(0)
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

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->SetActiveDragGrid(this, InventoryOperation);
	}
	else
	{
		SetActiveDragOperation(InventoryOperation);
	}

	int32 GridX = 0;
	int32 GridY = 0;
	if (CalculateDropGridPosition(InGeometry, InDragDropEvent, GridX, GridY) == false)
	{
		ClearHoveredCellState();
		UpdateDraggedPreviewState(InOperation, false);
		return true;
	}

	int32 PlacementGridX = GridX;
	int32 PlacementGridY = GridY;
	int32 PivotOffsetX = 0;
	int32 PivotOffsetY = 0;
	if (InventoryOperation->CalculateCurrentPivotOffset(PivotOffsetX, PivotOffsetY))
	{
		PlacementGridX -= PivotOffsetX;
		PlacementGridY -= PivotOffsetY;
	}

	SetHoveredCell(PlacementGridX, PlacementGridY);

	bool bCanPlace = false;
	if (InventoryComponent != nullptr)
	{
		bCanPlace = InventoryComponent->CanPlaceItem(
			InventoryOperation->GetItemInstanceId(),
			ContainerId,
			PlacementGridX,
			PlacementGridY,
			InventoryOperation->GetCurrentRotation());
	}

	UpdateDraggedPreviewState(InOperation, bCanPlace);
	return true;
}

void USMInventoryGridWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	USMInventoryDragDropOperation* InventoryOperation = GetInventoryDragDropOperation(InOperation);
	if (InventoryOperation == nullptr)
	{
		return;
	}

	if (ActiveDragDropOperation != InventoryOperation)
	{
		return;
	}

	ClearHoveredCellState();
	UpdateDraggedPreviewState(InOperation, false);
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
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		else
		{
			ClearActiveDragState();
		}

		return false;
	}

	int32 GridX = 0;
	int32 GridY = 0;
	if (CalculateDropGridPosition(InGeometry, InDragDropEvent, GridX, GridY) == false)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		else
		{
			ClearActiveDragState();
		}

		return false;
	}

	int32 PlacementGridX = GridX;
	int32 PlacementGridY = GridY;
	int32 PivotOffsetX = 0;
	int32 PivotOffsetY = 0;
	if (InventoryOperation->CalculateCurrentPivotOffset(PivotOffsetX, PivotOffsetY))
	{
		PlacementGridX -= PivotOffsetX;
		PlacementGridY -= PivotOffsetY;
	}

	int32 SourceQuickSlotIndex = INDEX_NONE;
	const bool bDraggedFromQuickSlot =
		InventoryComponent->GetQuickSlotIndexByContainerId(InventoryOperation->GetSourceContainerId(), SourceQuickSlotIndex);

	if (bDraggedFromQuickSlot)
	{
		if (ContainerId != InventoryComponent->GetMainInventory().ContainerId)
		{
			if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
			{
				OwningPanel->ClearActiveDragState();
			}
			else
			{
				ClearActiveDragState();
			}

			return false;
		}
	}

	const bool bCanPlace = InventoryComponent->CanPlaceItem(
		InventoryOperation->GetItemInstanceId(),
		ContainerId,
		PlacementGridX,
		PlacementGridY,
		InventoryOperation->GetCurrentRotation());

	if (bCanPlace == false)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		else
		{
			ClearActiveDragState();
		}

		return false;
	}

	ASMPlayerController* OwningPlayerController = GetOwningPlayer<ASMPlayerController>();
	if (OwningPlayerController == nullptr)
	{
		if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
		{
			OwningPanel->ClearActiveDragState();
		}
		else
		{
			ClearActiveDragState();
		}

		return false;
	}

	if (bDraggedFromQuickSlot)
	{
		OwningPlayerController->ServerRPCUnequipSkillFromQuickSlotToMainInventory(
			SourceQuickSlotIndex,
			PlacementGridX,
			PlacementGridY,
			InventoryOperation->GetCurrentRotation());
	}
	else
	{
		OwningPlayerController->ServerRPCMoveInventoryItem(
			InventoryOperation->GetItemInstanceId(),
			ContainerId,
			PlacementGridX,
			PlacementGridY,
			InventoryOperation->GetCurrentRotation());
	}

	if (USMPlayerInventoryPanelWidget* OwningPanel = GetTypedOuter<USMPlayerInventoryPanelWidget>())
	{
		OwningPanel->ClearActiveDragState();
	}
	else
	{
		ClearActiveDragState();
	}

	return true;
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
	FSMGridContainerState ContainerData;
	if (TryGetContainerData(ContainerData) == false)
	{
		GridWidth = 0;
		GridHeight = 0;
		ClearItemWidgets();
		ClearCellWidgets();
		ClearHoveredCellState();
		BP_OnGridRefreshed();
		return;
	}

	RefreshContainerSize(ContainerData);
	SyncGridStructure(ContainerData);
	SyncItemWidgets();
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
	HoveredItemInstanceId.Invalidate();
	HoveredGridX = -1;
	HoveredGridY = -1;
	UpdateCellStates();
}

void USMInventoryGridWidget::SetHoveredItemInstanceId(const FGuid& InItemInstanceId)
{
	if (HoveredItemInstanceId == InItemInstanceId)
	{
		return;
	}

	HoveredItemInstanceId = InItemInstanceId;
	UpdateCellStates();
}

void USMInventoryGridWidget::ClearActiveDragState()
{
	ActiveDragDropOperation = nullptr;
	ClearHoveredCellState();
}

void USMInventoryGridWidget::SetActiveDragOperation(USMInventoryDragDropOperation* InActiveDragDropOperation)
{
	ActiveDragDropOperation = InActiveDragDropOperation;
}

USMInventoryDragDropOperation* USMInventoryGridWidget::CreateDragDropOperationForItem(
	const FGuid& InItemInstanceId,
	int32 InPivotGridX,
	int32 InPivotGridY,
	FVector2D InPivotCellFraction)
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
				PreviewWidget->InitializePreviewFromInventory(BaseItemData.InstanceId, BaseItemData.Rotation, InventoryComponent);
			}
		}
	}

	int32 ShapeWidth = 1;
	int32 ShapeHeight = 1;
	if (const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(BaseItemData))
	{
		if (const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>())
		{
			if (GridShapeFragment->GetShapeMask().IsValidMaskData())
			{
				ShapeWidth = GridShapeFragment->GetShapeMask().Width;
				ShapeHeight = GridShapeFragment->GetShapeMask().Height;
			}
		}
	}

	int32 PivotRotatedLocalX = 0;
	int32 PivotRotatedLocalY = 0;
	if (InPivotGridX != INDEX_NONE && InPivotGridY != INDEX_NONE)
	{
		PivotRotatedLocalX = FMath::Max(0, InPivotGridX - BaseItemData.GridX);
		PivotRotatedLocalY = FMath::Max(0, InPivotGridY - BaseItemData.GridY);
	}

	int32 PivotShapeLocalX = PivotRotatedLocalX;
	int32 PivotShapeLocalY = PivotRotatedLocalY;

	switch (BaseItemData.Rotation)
	{
	case ESMGridRotation::Rot0:
		break;

	case ESMGridRotation::Rot90:
		PivotShapeLocalX = PivotRotatedLocalY;
		PivotShapeLocalY = ShapeHeight - 1 - PivotRotatedLocalX;
		break;

	case ESMGridRotation::Rot180:
		PivotShapeLocalX = ShapeWidth - 1 - PivotRotatedLocalX;
		PivotShapeLocalY = ShapeHeight - 1 - PivotRotatedLocalY;
		break;

	case ESMGridRotation::Rot270:
		PivotShapeLocalX = ShapeWidth - 1 - PivotRotatedLocalY;
		PivotShapeLocalY = PivotRotatedLocalX;
		break;

	default:
		break;
	}

	PivotShapeLocalX = FMath::Clamp(PivotShapeLocalX, 0, FMath::Max(0, ShapeWidth - 1));
	PivotShapeLocalY = FMath::Clamp(PivotShapeLocalY, 0, FMath::Max(0, ShapeHeight - 1));

	NewOperation->InitializeOperation(
		BaseItemData.InstanceId,
		BaseItemData.ParentContainerId,
		BaseItemData.GridX,
		BaseItemData.GridY,
		BaseItemData.Rotation,
		PivotShapeLocalX,
		PivotShapeLocalY,
		ShapeWidth,
		ShapeHeight,
		InPivotCellFraction,
		PreviewWidget);

	NewOperation->DefaultDragVisual = PreviewWidget;
	SetActiveDragOperation(NewOperation);
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

bool USMInventoryGridWidget::TryGetContainerData(FSMGridContainerState& OutContainerData) const
{
	if (InventoryComponent == nullptr)
	{
		return false;
	}

	return InventoryComponent->GetContainerData(ContainerId, OutContainerData);
}

void USMInventoryGridWidget::SyncGridStructure(const FSMGridContainerState& InContainerData)
{
	const bool bStructureChanged =
		CachedStructureContainerId != InContainerData.ContainerId ||
		CachedStructureWidth != InContainerData.ValidMask.Width ||
		CachedStructureHeight != InContainerData.ValidMask.Height ||
		CachedStructureBitMask != InContainerData.ValidMask.BitMask ||
		CellWidgets.Num() != (InContainerData.ValidMask.Width * InContainerData.ValidMask.Height);

	if (bStructureChanged == false)
	{
		return;
	}

	RebuildCellWidgets(InContainerData);
	CachedStructureContainerId = InContainerData.ContainerId;
	CachedStructureWidth = InContainerData.ValidMask.Width;
	CachedStructureHeight = InContainerData.ValidMask.Height;
	CachedStructureBitMask = InContainerData.ValidMask.BitMask;
}

void USMInventoryGridWidget::RebuildCellWidgets(const FSMGridContainerState& InContainerData)
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

	const FString& BitMask = InContainerData.ValidMask.BitMask;
	const int32 MaskWidth = InContainerData.ValidMask.Width;
	const int32 MaskHeight = InContainerData.ValidMask.Height;

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

void USMInventoryGridWidget::SyncItemWidgets()
{
	if (ItemLayerPanel == nullptr)
	{
		ClearItemWidgets();
		return;
	}

	if (ItemWidgetClass == nullptr)
	{
		ClearItemWidgets();
		return;
	}

	if (InventoryComponent == nullptr)
	{
		ClearItemWidgets();
		return;
	}

	TSet<FGuid> DesiredItemInstanceIds;
	TArray<FGuid> DesiredOwnerItemInstanceIds;
	TArray<FLinearColor> DesiredOccupiedAccentColors;
	DesiredOwnerItemInstanceIds.Init(FGuid(), CellWidgets.Num());
	DesiredOccupiedAccentColors.Init(FLinearColor::White, CellWidgets.Num());

	auto SyncItemWidget = [this, &DesiredItemInstanceIds, &DesiredOwnerItemInstanceIds, &DesiredOccupiedAccentColors](
		const FSMItemInstanceData& InBaseItemData)
	{
		DesiredItemInstanceIds.Add(InBaseItemData.InstanceId);

		USMItemWidget* ItemWidget = nullptr;
		if (TObjectPtr<USMItemWidget>* ExistingWidget = ItemWidgets.Find(InBaseItemData.InstanceId))
		{
			ItemWidget = ExistingWidget->Get();
		}

		if (ItemWidget == nullptr)
		{
			ItemWidget = CreateWidget<USMItemWidget>(this, ItemWidgetClass);
			if (ItemWidget == nullptr)
			{
				return;
			}

			ItemWidgets.Add(InBaseItemData.InstanceId, ItemWidget);
			ItemLayerPanel->AddChild(ItemWidget);
		}

		const bool bNeedsVisualRefresh =
			ItemWidget->GetOwningContainerId() != InBaseItemData.ParentContainerId ||
			ItemWidget->GetGridX() != InBaseItemData.GridX ||
			ItemWidget->GetGridY() != InBaseItemData.GridY ||
			ItemWidget->GetDisplayRotation() != InBaseItemData.Rotation ||
			ItemWidget->GetInventoryComponent() != InventoryComponent;

		if (bNeedsVisualRefresh)
		{
			ItemWidget->InitializeItemWidget(
				InBaseItemData.InstanceId,
				InBaseItemData.ParentContainerId,
				InBaseItemData.GridX,
				InBaseItemData.GridY,
				InBaseItemData.Rotation,
				InventoryComponent);
			ItemWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			ApplyItemWidgetLayout(ItemWidget, InBaseItemData.GridX, InBaseItemData.GridY);
		}

		TArray<FIntPoint> OccupiedCells;
		if (BuildOccupiedCellsFromItemData(InBaseItemData, OccupiedCells) == false)
		{
			return;
		}

		FLinearColor AccentColor = FLinearColor::White;
		GetItemAccentColor(InBaseItemData, AccentColor);

		for (const FIntPoint& OccupiedCell : OccupiedCells)
		{
			int32 CellArrayIndex = INDEX_NONE;
			if (TryGetCellArrayIndex(OccupiedCell.X, OccupiedCell.Y, CellArrayIndex) == false)
			{
				continue;
			}

			DesiredOwnerItemInstanceIds[CellArrayIndex] = InBaseItemData.InstanceId;
			DesiredOccupiedAccentColors[CellArrayIndex] = AccentColor;
		}
	};

	for (const FSMItemInstanceData& ItemData : InventoryComponent->GetItemEntries())
	{
		if (ItemData.ParentContainerId != ContainerId)
		{
			continue;
		}

		SyncItemWidget(ItemData);
	}

	for (const FSMSkillItemInstanceData& SkillData : InventoryComponent->GetSkillEntries())
	{
		if (SkillData.BaseItem.ParentContainerId != ContainerId)
		{
			continue;
		}

		SyncItemWidget(SkillData.BaseItem);
	}

	TArray<FGuid> StaleItemInstanceIds;
	for (const TPair<FGuid, TObjectPtr<USMItemWidget>>& Entry : ItemWidgets)
	{
		if (DesiredItemInstanceIds.Contains(Entry.Key))
		{
			continue;
		}

		StaleItemInstanceIds.Add(Entry.Key);
	}

	for (const FGuid& StaleItemInstanceId : StaleItemInstanceIds)
	{
		if (TObjectPtr<USMItemWidget>* StaleWidget = ItemWidgets.Find(StaleItemInstanceId))
		{
			if (StaleWidget->Get() != nullptr)
			{
				StaleWidget->Get()->RemoveFromParent();
			}
		}

		ItemWidgets.Remove(StaleItemInstanceId);
	}

	for (int32 CellIndex = 0; CellIndex < CellWidgets.Num(); ++CellIndex)
	{
		USMInventoryCellWidget* CellWidget = CellWidgets[CellIndex];
		if (CellWidget == nullptr)
		{
			continue;
		}

		const FGuid& DesiredOwnerItemInstanceId = DesiredOwnerItemInstanceIds[CellIndex];
		const FLinearColor& DesiredOccupiedAccentColor = DesiredOccupiedAccentColors[CellIndex];
		CellWidget->UpdateOccupiedItem(DesiredOwnerItemInstanceId, DesiredOccupiedAccentColor);
	}
}

void USMInventoryGridWidget::ClearCellWidgets()
{
	CellWidgets.Reset();
	CachedStructureContainerId.Invalidate();
	CachedStructureWidth = 0;
	CachedStructureHeight = 0;
	CachedStructureBitMask.Reset();

	if (CellLayerPanel == nullptr)
	{
		return;
	}

	CellLayerPanel->ClearChildren();
}

void USMInventoryGridWidget::ClearItemWidgets()
{
	ItemWidgets.Reset();

	if (ItemLayerPanel == nullptr)
	{
		return;
	}

	ItemLayerPanel->ClearChildren();
}

void USMInventoryGridWidget::UpdateCellStates()
{
	TArray<bool> HighlightedCellMask;
	bool bCanPlace = false;

	if (GridWidth > 0 && GridHeight > 0)
	{
		HighlightedCellMask.Init(false, GridWidth * GridHeight);
	}

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

		TArray<FIntPoint> HighlightedCells;
		FSMItemInstanceData BaseItemData;
		if (GetBaseItemData(ActiveDragDropOperation->GetItemInstanceId(), BaseItemData))
		{
			BuildOccupiedCells(
				BaseItemData,
				HoveredGridX,
				HoveredGridY,
				ActiveDragDropOperation->GetCurrentRotation(),
				HighlightedCells);
		}

		if (HighlightedCells.IsEmpty())
		{
			HighlightedCells.Add(FIntPoint(HoveredGridX, HoveredGridY));
		}

		for (const FIntPoint& HighlightedCell : HighlightedCells)
		{
			int32 CellArrayIndex = INDEX_NONE;
			if (TryGetCellArrayIndex(HighlightedCell.X, HighlightedCell.Y, CellArrayIndex) == false)
			{
				continue;
			}

			HighlightedCellMask[CellArrayIndex] = true;
		}
	}

	for (USMInventoryCellWidget* CellWidget : CellWidgets)
	{
		if (CellWidget == nullptr)
		{
			continue;
		}

		const bool bHovered =
			HoveredItemInstanceId.IsValid() &&
			CellWidget->GetOwnerItemInstanceId() == HoveredItemInstanceId;

		int32 CellArrayIndex = INDEX_NONE;
		const bool bHighlighted =
			TryGetCellArrayIndex(CellWidget->GetGridX(), CellWidget->GetGridY(), CellArrayIndex) &&
			HighlightedCellMask.IsValidIndex(CellArrayIndex) &&
			HighlightedCellMask[CellArrayIndex];
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

bool USMInventoryGridWidget::BuildOccupiedCells(const FSMItemInstanceData& InBaseItemData,
                                                int32 InGridX,
                                                int32 InGridY,
                                                ESMGridRotation InRotation,
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
		OutOccupiedCells.Add(FIntPoint(InGridX, InGridY));
		return true;
	}

	const USMGridShapeFragment* GridShapeFragment = ItemDefinition->FindFragmentByClass<USMGridShapeFragment>();
	if (GridShapeFragment == nullptr)
	{
		OutOccupiedCells.Add(FIntPoint(InGridX, InGridY));
		return true;
	}

	const FSMGridMaskData& ShapeMask = GridShapeFragment->GetShapeMask();
	if (ShapeMask.IsValidMaskData() == false)
	{
		OutOccupiedCells.Add(FIntPoint(InGridX, InGridY));
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

			switch (InRotation)
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

			OutOccupiedCells.Add(FIntPoint(InGridX + RotatedX, InGridY + RotatedY));
		}
	}

	return true;
}

bool USMInventoryGridWidget::BuildOccupiedCellsFromItemData(const FSMItemInstanceData& InBaseItemData,
                                                            TArray<FIntPoint>& OutOccupiedCells) const
{
	return BuildOccupiedCells(
		InBaseItemData,
		InBaseItemData.GridX,
		InBaseItemData.GridY,
		InBaseItemData.Rotation,
		OutOccupiedCells);
}

bool USMInventoryGridWidget::CalculateOccupiedCellBounds(const TArray<FIntPoint>& InOccupiedCells,
                                                         int32& OutMinGridX,
                                                         int32& OutMinGridY,
                                                         int32& OutMaxGridX,
                                                         int32& OutMaxGridY) const
{
	if (InOccupiedCells.IsEmpty())
	{
		return false;
	}

	OutMinGridX = InOccupiedCells[0].X;
	OutMinGridY = InOccupiedCells[0].Y;
	OutMaxGridX = InOccupiedCells[0].X;
	OutMaxGridY = InOccupiedCells[0].Y;

	for (int32 CellIndex = 1; CellIndex < InOccupiedCells.Num(); ++CellIndex)
	{
		const FIntPoint& OccupiedCell = InOccupiedCells[CellIndex];
		OutMinGridX = FMath::Min(OutMinGridX, OccupiedCell.X);
		OutMinGridY = FMath::Min(OutMinGridY, OccupiedCell.Y);
		OutMaxGridX = FMath::Max(OutMaxGridX, OccupiedCell.X);
		OutMaxGridY = FMath::Max(OutMaxGridY, OccupiedCell.Y);
	}

	return true;
}

bool USMInventoryGridWidget::GetItemAccentColor(const FSMItemInstanceData& InBaseItemData, FLinearColor& OutAccentColor) const
{
	OutAccentColor = FLinearColor::White;

	if (InventoryComponent == nullptr || InBaseItemData.InstanceId.IsValid() == false)
	{
		return false;
	}

	const USMItemDefinition* ItemDefinition = InventoryComponent->ResolveItemDefinition(InBaseItemData);
	if (ItemDefinition == nullptr)
	{
		return false;
	}

	const USMDisplayInfoFragment* DisplayInfoFragment = ItemDefinition->FindFragmentByClass<USMDisplayInfoFragment>();
	if (DisplayInfoFragment == nullptr)
	{
		return false;
	}

	OutAccentColor = DisplayInfoFragment->GetAccentColor();
	return true;
}

void USMInventoryGridWidget::ApplyItemOwnershipToCells(const FSMItemInstanceData& InBaseItemData)
{
	TArray<FIntPoint> OccupiedCells;
	if (BuildOccupiedCellsFromItemData(InBaseItemData, OccupiedCells) == false)
	{
		return;
	}

	FLinearColor AccentColor = FLinearColor::White;
	GetItemAccentColor(InBaseItemData, AccentColor);

	for (const FIntPoint& OccupiedCell : OccupiedCells)
	{
		USMInventoryCellWidget* CellWidget = FindCellWidget(OccupiedCell.X, OccupiedCell.Y);
		if (CellWidget == nullptr)
		{
			continue;
		}

	CellWidget->UpdateOccupiedItem(InBaseItemData.InstanceId, AccentColor);
	}
}

void USMInventoryGridWidget::ClearCellOccupancyState()
{
	for (USMInventoryCellWidget* CellWidget : CellWidgets)
	{
		if (CellWidget == nullptr)
		{
			continue;
		}

		CellWidget->UpdateOccupiedItem(FGuid(), FLinearColor::White);
	}
}

void USMInventoryGridWidget::RefreshContainerSize(const FSMGridContainerState& InContainerData)
{
	GridWidth = 0;
	GridHeight = 0;
	GridWidth = InContainerData.ValidMask.Width;
	GridHeight = InContainerData.ValidMask.Height;
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

	FSMItemInstanceData BaseItemData;
	if (GetBaseItemData(InItemWidget->GetItemInstanceId(), BaseItemData))
	{
		TArray<FIntPoint> OccupiedCells;
		if (BuildOccupiedCells(
			BaseItemData,
			InGridX,
			InGridY,
			InItemWidget->GetDisplayRotation(),
			OccupiedCells))
		{
			CalculateOccupiedCellBounds(OccupiedCells, MinGridX, MinGridY, MaxGridX, MaxGridY);
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

bool USMInventoryGridWidget::TryGetCellArrayIndex(int32 InGridX, int32 InGridY, int32& OutCellArrayIndex) const
{
	OutCellArrayIndex = INDEX_NONE;

	if (GridWidth <= 0 || GridHeight <= 0)
	{
		return false;
	}

	if (InGridX < 0 || InGridY < 0 || InGridX >= GridWidth || InGridY >= GridHeight)
	{
		return false;
	}

	OutCellArrayIndex = (InGridY * GridWidth) + InGridX;
	return true;
}
