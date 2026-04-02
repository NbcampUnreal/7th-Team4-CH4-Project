#include "SMEnemyHPBarComponent.h"
#include "UI/SMEnemyHPBarWidget.h"
#include "TimerManager.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"


USMEnemyHPBarComponent::USMEnemyHPBarComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    SetWidgetSpace(EWidgetSpace::World);
    SetDrawAtDesiredSize(true);
}


void USMEnemyHPBarComponent::BeginPlay()
{
    Super::BeginPlay();
    SetVisibility(false);

    /** GAS - 데이터 받은 후 작업 */
}


void USMEnemyHPBarComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GetNetMode() == NM_DedicatedServer) return;

    if (IsVisible())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->PlayerCameraManager)
        {
            FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
            FVector widgetLocation = GetComponentLocation();

            FRotator NewRot = (CameraLocation - widgetLocation).Rotation();
            SetWorldRotation(NewRot);
        }
    }
}

void USMEnemyHPBarComponent::HideHPBar()
{
    SetVisibility(false);
}
