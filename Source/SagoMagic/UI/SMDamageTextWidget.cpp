#include "SMDamageTextWidget.h"
#include "Components/TextBlock.h"

void USMDamageTextWidget::SetDamageText(float DamageAmount)
{
	if (TextBlock_Damage)
	{
		int32 DamageInt = FMath::RoundToInt(DamageAmount);
		TextBlock_Damage->SetText(FText::AsNumber(DamageInt));
	}
}