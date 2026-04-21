#include "SMGoldTextWidget.h"
#include "Components/TextBlock.h"

void USMGoldTextWidget::SetGoldText(float GoldDelta)
{
	if (!TextBlockGold) return;
	
	int32 Amount = FMath::Abs(FMath::RoundToInt(GoldDelta));
	FString GoldString = GoldDelta >= 0.f 
	? FString::Printf(TEXT("+%dG"), Amount)
	: FString::Printf(TEXT("-%dG"),Amount);
	
	TextBlockGold->SetText(FText::FromString(GoldString));
	
	FSlateColor Color = GoldDelta >= 0.f
		? FSlateColor(FLinearColor(1.f, 0.85f, 0.f))
		: FSlateColor(FLinearColor(1.f, 0.2f, 0.2f));
	
	TextBlockGold->SetColorAndOpacity(Color);
}
