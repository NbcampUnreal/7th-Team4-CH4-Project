#include "SMSkillCooldownWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USMSkillCooldownWidget::InitializeWithASC(UAbilitySystemComponent* InASC, FGameplayTag InCooldownTag)
{
    UnbindASC();
    BoundASC = nullptr;
    CooldownTag = FGameplayTag();
    bOnCooldown = false;
    TotalCooldown = 0.f;

    if (ProgressBar_Cooldown)
    {
        ProgressBar_Cooldown->SetPercent(0.f);
    }
    SetVisibility(ESlateVisibility::Collapsed);
    
    if (!InASC || !InCooldownTag.IsValid()) return;
    
    BoundASC = InASC;
    CooldownTag = InCooldownTag;
    
    BoundASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &USMSkillCooldownWidget::OnCooldownTagChanged);
    
    OnCooldownTagChanged(CooldownTag, BoundASC->GetTagCount(CooldownTag));
    
    if (!bOnCooldown) // 쿨다운 태그 적용 확인
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void USMSkillCooldownWidget::NativeDestruct()
{
    UnbindASC();
    Super::NativeDestruct();
}

void USMSkillCooldownWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    
    if (!bOnCooldown || !BoundASC || TotalCooldown <= 0.f) return;

    FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
        FGameplayTagContainer(CooldownTag));
    
    // 시간 가져오기
    TArray<float> Remaining = BoundASC->GetActiveEffectsTimeRemaining(Query);
    
    if (Remaining.Num() > 0 && Remaining[0] > 0.f)
    {
        const float Percent = FMath::Clamp(Remaining[0] / TotalCooldown, 0.f, 1.f);
        
        if (ProgressBar_Cooldown)
        {
            ProgressBar_Cooldown->SetPercent(Percent);
        }
        
        if (TextBlock_Cooldown)
        {
            FString TimeString = FString::Printf(TEXT("%.1f"), Remaining[0]);
            TextBlock_Cooldown->SetText(FText::FromString(TimeString));
        }
    }
    else
    {
        bOnCooldown = false;
        
        if (ProgressBar_Cooldown) 
        {
            ProgressBar_Cooldown->SetPercent(0.f);
        }
        
        SetVisibility(ESlateVisibility::Collapsed); 
    }
}

void USMSkillCooldownWidget::OnCooldownTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    (void)Tag; 

    if (NewCount > 0 && BoundASC)
    {
        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(CooldownTag));

        TArray<FActiveGameplayEffectHandle> Handles = BoundASC->GetActiveEffects(Query);
        if (Handles.Num() > 0)
        {
            float StartTime = 0.f, Duration = 0.f;
            BoundASC->GetGameplayEffectStartTimeAndDuration(Handles[0], StartTime, Duration);

            TotalCooldown = Duration;
            bOnCooldown = true;
            
            SetVisibility(ESlateVisibility::HitTestInvisible); 
        }
    }
    else
    {
        bOnCooldown = false;
        
        if (ProgressBar_Cooldown)
        {
            ProgressBar_Cooldown->SetPercent(0.f);
        }
        SetVisibility(ESlateVisibility::Collapsed);
    }
}

void USMSkillCooldownWidget::UnbindASC()
{
    if (BoundASC && CooldownTag.IsValid())
    {
        BoundASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
            .RemoveAll(this);
    }
    BoundASC = nullptr;
}