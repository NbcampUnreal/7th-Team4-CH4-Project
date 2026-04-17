#include "SMSkillCooldownWidget.h"
#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USMSkillCooldownWidget::InitializeWithASC(UAbilitySystemComponent* InASC, FGameplayTag InCooldownTag)
{
    if (!InASC || !InCooldownTag.IsValid()) return;

    UnbindASC();
    BoundASC = InASC;
    CooldownTag = InCooldownTag;

    // 쿨타임 태그 추가, 제거 감지 바인딩
    BoundASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved)
        .AddUObject(this, &USMSkillCooldownWidget::OnCooldownTagChanged);

    // 초기 상태 반영 - 현재 쿨다운 태그가 있는지 확인
    OnCooldownTagChanged(CooldownTag, BoundASC->GetTagCount(CooldownTag));
    
    if (!bOnCooldown)
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

    TArray<float> Remaining = BoundASC->GetActiveEffectsTimeRemaining(Query);

    // 쿨다운이 아직 남아있는 경우
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
            RemainingCooldown = Duration;
            bOnCooldown = true;
            
            SetVisibility(ESlateVisibility::HitTestInvisible); 
        }
    }
    else
    {
        bOnCooldown = false;
        RemainingCooldown = 0.f;
        
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