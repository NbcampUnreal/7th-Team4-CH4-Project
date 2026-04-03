#include "SMPlayerHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"

void USMPlayerHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (TextBlock_PlayerHP) HPTextTemplate = TextBlock_PlayerHP->GetText();
	
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetPawn())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
		if (ASC)
		{
			HealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetHealthAttribute())
			.AddUObject(this, &USMPlayerHPBarWidget::OnHealthChanged);
			MaxHealthChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetMaxHealthAttribute())
			.AddUObject(this, &USMPlayerHPBarWidget::OnMaxHealthChanged);
			
			// 초기값 캐싱
			CachedMaxHP = ASC->GetNumericAttribute(USMPlayerAttributeSet::GetMaxHealthAttribute());
			CachedCurrentHP = ASC->GetNumericAttribute(USMPlayerAttributeSet::GetHealthAttribute());
			
			UpdateHPText(CachedCurrentHP, CachedMaxHP);
			
			if (CachedCurrentHP > 0.0f)
			{
				TargetPercent = CachedCurrentHP / CachedMaxHP;
				CurrentPercent = TargetPercent;
				if (ProgressBar_PlayerHP) ProgressBar_PlayerHP->SetPercent(CurrentPercent);
			}
		}
	}
}

void USMPlayerHPBarWidget::NativeDestruct()
{
	// 구독 해제
	UAbilitySystemComponent* ASC = GetPlayerASC();
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetHealthAttribute())
		.Remove(HealthChangedHandle);
		ASC->GetGameplayAttributeValueChangeDelegate(USMPlayerAttributeSet::GetMaxHealthAttribute())
		.Remove(MaxHealthChangedHandle);
	}
	
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

void USMPlayerHPBarWidget::UpdateHPText(float CurrentHP, float MaxHP)
{
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

	UpdateHPText(CachedCurrentHP, CachedMaxHP);
}

void USMPlayerHPBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	CachedMaxHP = Data.NewValue;
    
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetPawn())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
		if (ASC && CachedMaxHP > 0.0f)
		{
			CachedCurrentHP = ASC->GetNumericAttribute(USMPlayerAttributeSet::GetHealthAttribute());
			TargetPercent = FMath::Clamp(CachedCurrentHP / CachedMaxHP, 0.0f, 1.0f);
            
			// 나중에 최대 체력이 변했을 때도 텍스트 갱신
			UpdateHPText(CachedCurrentHP, CachedMaxHP);
		}
	}
}

UAbilitySystemComponent* USMPlayerHPBarWidget::GetPlayerASC() const
{
	APlayerController* PC = GetOwningPlayer();
	if (PC && PC->GetPawn())
	{
		return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(PC->GetPawn());
	}
	return nullptr;
}
