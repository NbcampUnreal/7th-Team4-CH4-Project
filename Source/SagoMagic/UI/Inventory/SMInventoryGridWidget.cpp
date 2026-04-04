#include "UI/Inventory/SMInventoryGridWidget.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Blueprint/DragDropOperation.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMContainerTypes.h"
#include "Inventory/Core/SMItemInstanceTypes.h"

#include "UI/Inventory/SMInventoryCellWidget.h"
#include "UI/Inventory/SMItemWidget.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"

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
			GridY);
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

	InventoryComponent->SetItemRotation(
		InventoryOperation->GetItemInstanceId(),
		InventoryOperation->GetCurrentRotation());

	const bool bMoveSucceeded = InventoryComponent->MoveItem(
		InventoryOperation->GetItemInstanceId(),
		ContainerId,
		GridX,
		GridY);

	ActiveDragDropOperation = nullptr;

	if (bMoveSucceeded)
	{
		RefreshGrid();
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
		return;
	}

	ActiveDragDropOperation->UpdateCurrentRotation(NextRotation);

	USMDragItemPreviewWidget* PreviewWidget = ActiveDragDropOperation->GetDragPreviewWidget();
	if (PreviewWidget != nullptr)
	{
		PreviewWidget->UpdatePreviewRotation(NextRotation);
	}

	if (HoveredGridX >= 0 && HoveredGridY >= 0 && InventoryComponent != nullptr)
	{
		const bool bCanPlace = InventoryComponent->CanPlaceItem(
			ActiveDragDropOperation->GetItemInstanceId(),
			ContainerId,
			HoveredGridX,
			HoveredGridY);

		UpdateDraggedPreviewState(ActiveDragDropOperation, bCanPlace);
	}
}

void USMInventoryGridWidget::ClearHoveredCellState()
{
	HoveredGridX = -1;
	HoveredGridY = -1;
	UpdateCellStates();
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

		ItemLayerPanel->AddChild(NewItemWidget);
		ApplyItemWidgetLayout(NewItemWidget, ItemData.GridX, ItemData.GridY);
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

		ItemLayerPanel->AddChild(NewItemWidget);
		ApplyItemWidgetLayout(NewItemWidget, SkillData.BaseItem.GridX, SkillData.BaseItem.GridY);
	}
}

void USMInventoryGridWidget::ClearCellWidgets()
{
	CellWidgets.Reset();

	if (CellLayerPanel != nullptr)
	{
		CellLayerPanel->ClearChildren();
	}
}

void USMInventoryGridWidget::ClearItemWidgets()
{
	if (ItemLayerPanel != nullptr)
	{
		ItemLayerPanel->ClearChildren();
	}
}

void USMInventoryGridWidget::UpdateCellStates()
{
	for (USMInventoryCellWidget* CellWidget : CellWidgets)
	{
		if (CellWidget == nullptr)
		{
			continue;
		}

		const bool bHovered =
			CellWidget->GetGridX() == HoveredGridX &&
			CellWidget->GetGridY() == HoveredGridY;

		CellWidget->UpdateCellState(
			CellWidget->IsCellEnabled(),
			bHovered,
			false,
			false);
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
}
