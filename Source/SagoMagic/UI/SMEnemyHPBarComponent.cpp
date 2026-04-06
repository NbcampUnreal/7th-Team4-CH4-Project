#include "SMEnemyHPBarComponent.h"
#include "UI/SMEnemyHPBarWidget.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"


USMEnemyHPBarComponent::USMEnemyHPBarComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    SetWidgetSpace(EWidgetSpace::World);
    SetDrawAtDesiredSize(false);
}

void USMEnemyHPBarComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetVisibility(false);
}


void USMEnemyHPBarComponent::InitializeHPBar(UAbilitySystemComponent* InASC)
{
    if (InASC)
    {
        ASC = InASC;
        
        // ASC 체력 델리게이트 바인딩
        ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
        .AddUObject(this, &USMEnemyHPBarComponent::OnHPChanged);
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
    if (IsValid(this) && IsValid(GetOwner()))
    {
        SetVisibility(false);
    }
}

void USMEnemyHPBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ASC) // 등록했던 체력 변경 델리게이트 해제
    {
        ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
        .RemoveAll(this);
    }
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
    }
    
    Super::EndPlay(EndPlayReason);
}