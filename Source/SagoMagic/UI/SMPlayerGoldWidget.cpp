#include "SMPlayerGoldWidget.h"
#include "Components/TextBlock.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"


void USMPlayerGoldWidget::InitializeWithASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;

	UnbindASC();
	BoundASC = InASC;

	if (BoundASC.IsValid())
	{
		// GAS Attribute 변경 시 호출될 델리게이트 연결
		GoldChangedHandle = BoundASC.Get()->GetGameplayAttributeValueChangeDelegate(
			USMPlayerAttributeSet::GetGoldAttribute()).AddUObject(this, &USMPlayerGoldWidget::OnGoldChanged);

		// 현재 골드 즉시 표시
		float CurrentGold = BoundASC.Get()->GetNumericAttribute(USMPlayerAttributeSet::GetGoldAttribute());
		UpdateGoldText(CurrentGold);
	}
}

void USMPlayerGoldWidget::NativeDestruct()
{
	UnbindASC();
	Super::NativeDestruct();
}

void USMPlayerGoldWidget::OnGoldChanged(const FOnAttributeChangeData& Data)
{
	UpdateGoldText(Data.NewValue);
}

void USMPlayerGoldWidget::UpdateGoldText(float CurrentGold)
{
	if (TextBlock_Gold)
	{
		FString GoldString = FString::Printf(TEXT("%d"), FMath::RoundToInt(CurrentGold));
		TextBlock_Gold->SetText(FText::FromString(GoldString));
	}
}

void USMPlayerGoldWidget::UnbindASC()
{
	// BoundASC가 유효할 때만 구독 해제
	if (BoundASC.IsValid())
	{
		BoundASC.Get()->GetGameplayAttributeValueChangeDelegate(
			USMPlayerAttributeSet::GetGoldAttribute()).Remove(GoldChangedHandle);
	}
	BoundASC.Reset();
}
