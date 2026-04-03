#include "UI/Inventory/SMInventoryRootWidget.h"

#include "Components/PanelWidget.h"

#include "Inventory/Components/SMInventoryComponent.h"
#include "UI/Inventory/SMPlayerInventoryPanelWidget.h"

USMInventoryRootWidget::USMInventoryRootWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , PanelLayer(nullptr)
	  , InventoryComponent(nullptr)
	  , CurrentPanelWidget(nullptr)
{
}

void USMInventoryRootWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USMInventoryRootWidget::InitializeRootWidget(USMInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;
	CreateCurrentPanelWidget();
	RefreshRootWidget();
}

USMPlayerInventoryPanelWidget* USMInventoryRootWidget::CreateCurrentPanelWidget()
{
	ClearCurrentPanelWidget();

	if (PlayerInventoryPanelWidgetClass == nullptr)
	{
		return nullptr;
	}

	if (InventoryComponent == nullptr)
	{
		return nullptr;
	}

	CurrentPanelWidget = CreateWidget<USMPlayerInventoryPanelWidget>(this, PlayerInventoryPanelWidgetClass);
	if (CurrentPanelWidget == nullptr)
	{
		return nullptr;
	}

	CurrentPanelWidget->InitializePanelWidget(InventoryComponent);

	if (PanelLayer != nullptr)
	{
		PanelLayer->AddChild(CurrentPanelWidget);
	}

	return CurrentPanelWidget;
}

void USMInventoryRootWidget::ClearCurrentPanelWidget()
{
	if (CurrentPanelWidget == nullptr)
	{
		return;
	}

	CurrentPanelWidget->RemoveFromParent();
	CurrentPanelWidget = nullptr;
}

void USMInventoryRootWidget::RefreshRootWidget()
{
	if (CurrentPanelWidget != nullptr)
	{
		CurrentPanelWidget->RefreshPanel();
	}

	BP_OnRootWidgetRefreshed();
}
