#include "UI/Inventory/SMItemWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "UI/Inventory/SMInventoryDragDropOperation.h"
#include "UI/Inventory/SMDragItemPreviewWidget.h"

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
	if (CanStartDrag() == false)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
}

void USMItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
                                         UDragDropOperation*& OutOperation)
{
	OutOperation = CreateDragDropOperation();
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

USMInventoryDragDropOperation* USMItemWidget::CreateDragDropOperation()
{
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
		PreviewWidget);

	NewOperation->DefaultDragVisual = PreviewWidget;

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

	PreviewWidget->InitializePreview(ItemInstanceId, DisplayRotation);
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
