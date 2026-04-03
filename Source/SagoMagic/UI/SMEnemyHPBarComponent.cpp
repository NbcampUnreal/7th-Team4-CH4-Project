#include "SMEnemyHPBarComponent.h"
#include "UI/SMEnemyHPBarWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "TimerManager.h"


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
    
    // GAS - Enemy에서 ASC 가져오기
    AActor* OwnerActor = GetOwner();
    if (OwnerActor)
    {
        ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor);
        
        if (ASC)
        {
            ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
               .AddUObject(this, &USMEnemyHPBarComponent::OnHPChanged);
        }
    }
}


void USMEnemyHPBarComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 서버에서는 UI 렌더링 처리할 필요 X라서 추가하는 코드
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

void USMEnemyHPBarComponent::OnHPChanged(const FOnAttributeChangeData& Data)
{
    SetVisibility(true);
    
    USMEnemyHPBarWidget* HPWidget = Cast<USMEnemyHPBarWidget>(GetUserWidgetObject());
    if (HPWidget && ASC)
    {
        float CurrentHP = Data.NewValue;
        float MaxHealth = ASC->GetNumericAttribute(USMMonsterAttributeSet::GetMaxHealthAttribute());
        float HPPercent = (MaxHealth > 0.0f) ? (CurrentHP / MaxHealth) : 0.0f;
        
        HPWidget->UpdateHPBar(HPPercent);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &USMEnemyHPBarComponent::HideHPBar,
            DisplayDuration, false);
    }
}

void USMEnemyHPBarComponent::HideHPBar()
{
    SetVisibility(false);
}

void USMEnemyHPBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ASC)
    {
        // 등록했던 체력 변경 델리게이트 해제
        ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute()).RemoveAll(this);
    }
    
    Super::EndPlay(EndPlayReason);
}