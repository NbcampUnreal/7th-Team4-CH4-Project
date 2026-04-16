#include "Components/SMInteractionWorldWidgetComponent.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Inventory/SMInteractionWorldInfoWidget.h"
#include "Blueprint/UserWidget.h"

USMInteractionWorldWidgetComponent::USMInteractionWorldWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(true);

	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(false);
	SetDrawSize(FIntPoint(1000, 600));
	SetPivot(FVector2D(0.5f, 1.0f));
	SetTwoSided(true);
	SetBlendMode(EWidgetBlendMode::Transparent);
	SetHiddenInGame(true);
	SetVisibility(false);
}

void USMInteractionWorldWidgetComponent::BeginPlay()
{
	Super::BeginPlay();

	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
	SetDrawSize(FVector2D(360.0f, 180.0f));
	SetPivot(FVector2D(0.5f, 1.0f));
	SetTwoSided(true);
	SetBlendMode(EWidgetBlendMode::Transparent);
	SetHiddenInGame(true);

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (PlayerController->IsLocalController())
		{
			SetOwnerPlayer(PlayerController->GetLocalPlayer());
		}
	}

	HideInteractionInfo();
}

USMInteractionWorldInfoWidget* USMInteractionWorldWidgetComponent::GetInteractionWorldInfoWidget() const
{
	USMInteractionWorldInfoWidget* InteractionWidget = Cast<USMInteractionWorldInfoWidget>(GetUserWidgetObject());
	return InteractionWidget;
}

void USMInteractionWorldWidgetComponent::ShowInteractionInfo(const FSMInteractionWorldInfoData& InDisplayData)
{
	SetHiddenInGame(false);
	SetVisibility(true);
	if (USMInteractionWorldInfoWidget* InteractionInfoWidget = GetInteractionWorldInfoWidget())
	{
		InteractionInfoWidget->ShowInteractionInfo(InDisplayData);
		InteractionInfoWidget->InvalidateLayoutAndVolatility();
		InteractionInfoWidget->ForceLayoutPrepass();
	}
	RequestRedraw();
}

void USMInteractionWorldWidgetComponent::HideInteractionInfo()
{
	SetVisibility(false);
	SetHiddenInGame(true);
	RequestRedraw();
}
