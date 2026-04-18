#include "SMBuildingHPBarComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "UI/SMBuildingHPBarWidget.h"
#include "GAS/AttributeSets/SMBuildingAttributeSet.h"
#include "GAS/AttributeSets/SMBaseCampAttributeSet.h"
#include "Kismet/GameplayStatics.h"

USMBuildingHPBarComponent::USMBuildingHPBarComponent()
{
    PrimaryComponentTick.bCanEverTick = true; 
    
    SetWidgetSpace(EWidgetSpace::World);
    SetDrawAtDesiredSize(true);
}

void USMBuildingHPBarComponent::BeginPlay()
{
    Super::BeginPlay();
    
    SetVisibility(false);
    
    bIsVisibleFromDamage = false;
    bIsVisibleFromProximity = false;
    UpdateVisibility();
    
    TryInitASC();
    
    GetWorld()->GetTimerManager().SetTimer(
        DistanceCheckTimerHandle, 
        this, 
        &USMBuildingHPBarComponent::CheckDistanceToPlayer, 
        DistanceCheckInterval, 
        true);
}

void USMBuildingHPBarComponent::TryInitASC()
{
    if (ASC) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner);
    if (!ASI) return;
    
    UAbilitySystemComponent* FoundASC = ASI->GetAbilitySystemComponent();
    
    if (FoundASC && GetUserWidgetObject())
    {
        InitializeHPBar(FoundASC);
        
        GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
        return;
    }
    
    GetWorld()->GetTimerManager().SetTimer(
        ASC_InitTimerHandle, this, &USMBuildingHPBarComponent::TryInitASC, 0.1f, false);
}

void USMBuildingHPBarComponent::InitializeHPBar(UAbilitySystemComponent* InASC)
{
    if (!InASC) return;
    
    ASC = InASC;

    FGameplayAttribute HealthAttr;
    FGameplayAttribute MaxHealthAttr;
    
    if (ASC->GetSet<USMBuildingAttributeSet>())
    {
        HealthAttr    = USMBuildingAttributeSet::GetHealthAttribute();
        MaxHealthAttr = USMBuildingAttributeSet::GetMaxHealthAttribute();
    }
    else if (ASC->GetSet<USMBaseCampAttributeSet>())
    {
        HealthAttr    = USMBaseCampAttributeSet::GetHealthAttribute();
        MaxHealthAttr = USMBaseCampAttributeSet::GetMaxHealthAttribute();
    }
    else
    {
        return;
    }
    
    ASC->GetGameplayAttributeValueChangeDelegate(HealthAttr)
        .AddUObject(this, &USMBuildingHPBarComponent::OnHPChanged);
    
    if (USMBuildingHPBarWidget* HPBarWidget = Cast<USMBuildingHPBarWidget>(GetUserWidgetObject()))
    {
        const float CurrentHP = ASC->GetNumericAttribute(HealthAttr);
        const float MaxHP     = ASC->GetNumericAttribute(MaxHealthAttr);
        
        HPBarWidget->SetMaxHP(MaxHP);
        HPBarWidget->UpdateHPBar(CurrentHP, MaxHP);
    }
}

void USMBuildingHPBarComponent::OnHPChanged(const FOnAttributeChangeData& Data)
{
    bIsVisibleFromDamage = true;
    UpdateVisibility();
    
    GetWorld()->GetTimerManager().SetTimer(
        HideTimerHandle, this, &USMBuildingHPBarComponent::HideHPBar, DisplayDuration, false);

    float MaxHP = 1.f;
    
    if (ASC)
    {
        if (ASC->GetSet<USMBuildingAttributeSet>())
            MaxHP = ASC->GetNumericAttribute(USMBuildingAttributeSet::GetMaxHealthAttribute());
        else if (ASC->GetSet<USMBaseCampAttributeSet>())
            MaxHP = ASC->GetNumericAttribute(USMBaseCampAttributeSet::GetMaxHealthAttribute());
    }
    
    if (USMBuildingHPBarWidget* HPBarWidget = Cast<USMBuildingHPBarWidget>(GetUserWidgetObject()))
    {
        HPBarWidget->UpdateHPBar(Data.NewValue, MaxHP);
    }
}

void USMBuildingHPBarComponent::HideHPBar()
{
    bIsVisibleFromDamage = false;
    
    UpdateVisibility();
}

void USMBuildingHPBarComponent::CheckDistanceToPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn) return;
    
    const float DistanceToPlayer = FVector::Dist(GetComponentLocation(), PlayerPawn->GetActorLocation());
    const bool bIsCloseEnough = (DistanceToPlayer <= VisibleDistance);
    
    if (bIsVisibleFromProximity != bIsCloseEnough)
    {
        bIsVisibleFromProximity = bIsCloseEnough;
        UpdateVisibility();
    }
}

void USMBuildingHPBarComponent::UpdateVisibility()
{
    if (IsValid(this) && IsValid(GetOwner()))
    {
        SetVisibility(bIsVisibleFromDamage || bIsVisibleFromProximity);
    }
}

void USMBuildingHPBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ASC)
    {
        if (ASC->GetSet<USMBuildingAttributeSet>())
        {
            ASC->GetGameplayAttributeValueChangeDelegate(USMBuildingAttributeSet::GetHealthAttribute())
                .RemoveAll(this);
        }
        else if (ASC->GetSet<USMBaseCampAttributeSet>())
        {
            ASC->GetGameplayAttributeValueChangeDelegate(USMBaseCampAttributeSet::GetHealthAttribute())
                .RemoveAll(this);
        }
    }
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(DistanceCheckTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}