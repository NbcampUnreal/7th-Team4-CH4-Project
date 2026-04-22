#include "SMDamageTextWidget.h"
#include "Components/TextBlock.h"

void USMDamageTextWidget::SetDamageText(float DamageAmount)
{
	if (TextBlock_Damage)
	{
		int32 DamageInt = FMath::RoundToInt(DamageAmount);
		TextBlock_Damage->SetText(FText::AsNumber(DamageInt));
		const float DamageScale = CalculateDamageTextScale(DamageAmount);
		TextBlock_Damage->SetRenderScale(FVector2D(DamageScale, DamageScale));
	}
}

float USMDamageTextWidget::CalculateDamageTextScale(float DamageAmount) const
{
	const float ClampedDamageAmount = FMath::Max(0.0f, DamageAmount);
	if (ClampedDamageAmount < 100.0f)
	{
		return 1.0f;
	}

	return 1.5f + FMath::LogX(10.0f, ClampedDamageAmount / 100.0f);
}
