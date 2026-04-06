#include "SMPlayerHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"


void USMPlayerHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (TextBlock_PlayerHP)
	{
		HPTextTemplate = TextBlock_PlayerHP->GetText();
	}
}

void USMPlayerHPBarWidget::InitializeWithASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;
	
	UnbindASC();
	BoundASC = InASC;
	
	// GAS Attribute 변경 시 호출될 델리게이트 연결
	HealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetHealthAttribute())
	.AddUObject(this, &USMPlayerHPBarWidget::OnHealthChanged);
	MaxHealthChangedHandle = BoundASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetMaxHealthAttribute())
	.AddUObject(this, &USMPlayerHPBarWidget::OnMaxHealthChanged);
			
	// 초기값 캐싱
	CachedMaxHP = BoundASC->GetNumericAttribute(USMPlayerAttributeSet::GetMaxHealthAttribute());
	CachedCurrentHP = BoundASC->GetNumericAttribute(USMPlayerAttributeSet::GetHealthAttribute());
	
	TargetPercent = (CachedMaxHP > 0.f) ? FMath::Clamp(CachedCurrentHP / CachedMaxHP, 0.f, 1.f) : 0.f;
	CurrentPercent = TargetPercent;

	UpdateHPBar(CachedCurrentHP, CachedMaxHP);
	
	if (ProgressBar_PlayerHP)
	{
		ProgressBar_PlayerHP->SetPercent(CurrentPercent);
	}
}

void USMPlayerHPBarWidget::UnbindASC()
{
	if (!BoundASC) return;

	BoundASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetHealthAttribute())
	.Remove(HealthChangedHandle);
	BoundASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetMaxHealthAttribute())
	.Remove(MaxHealthChangedHandle);

	BoundASC = nullptr;
}

void USMPlayerHPBarWidget::NativeDestruct()
{
	UnbindASC();
	Super::NativeDestruct();
}

void USMPlayerHPBarWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	
	// 보간 로직
	if (!FMath::IsNearlyEqual(CurrentPercent, TargetPercent, 0.001f))
	{
		CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, DeltaTime, InterpSpeed);
		if (ProgressBar_PlayerHP)
		{
			ProgressBar_PlayerHP->SetPercent(CurrentPercent);
		}
	}
}

void USMPlayerHPBarWidget::UpdateHPBar(float CurrentHP, float MaxHP)
{
	// 텍스트 업데이트 로직
	if (TextBlock_PlayerHP && !HPTextTemplate.IsEmpty())
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("CurrentHP"), FText::AsNumber(FMath::RoundToInt(CurrentHP)));
		Args.Add(TEXT("MaxHP"), FText::AsNumber(FMath::RoundToInt(MaxHP)));
		FText FormattedText = FText::Format(HPTextTemplate, Args);
		TextBlock_PlayerHP->SetText(FormattedText);
	}
}

void USMPlayerHPBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedCurrentHP = Data.NewValue; // 현재 값 업데이트
    
	if (CachedMaxHP > 0.0f)
	{
		TargetPercent = FMath::Clamp(CachedCurrentHP / CachedMaxHP, 0.0f, 1.0f);
	}
	UpdateHPBar(CachedCurrentHP, CachedMaxHP);
}

void USMPlayerHPBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxHP = Data.NewValue;
    if (!BoundASC) return;
	
	CachedCurrentHP = BoundASC->GetNumericAttribute(USMPlayerAttributeSet::GetHealthAttribute());
	TargetPercent = FMath::Clamp(CachedCurrentHP / CachedMaxHP, 0.0f, 1.0f);
            
	// 나중에 최대 체력이 변했을 때도 텍스트 갱신
	UpdateHPBar(CachedCurrentHP, CachedMaxHP);
}