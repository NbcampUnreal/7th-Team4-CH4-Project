#include "SMPlayerDeathWidget.h"
#include "Components/TextBlock.h"

void USMPlayerDeathWidget::ShowDeathWidget(float InRespawnTime)
{
	RemainingTime = FMath::Max(0.0f, InRespawnTime);
	bCountingDown = true;

	if (TextBlock_DeathMessage)
	{
		TextBlock_DeathMessage->SetText(DeathMessageText);
	}

	UpdateCountdownText(); // 초기 남은 시간
	SetVisibility(ESlateVisibility::Visible);

	BP_OnDied();
}

void USMPlayerDeathWidget::HideDeathWidget()
{
	bCountingDown = false;
	RemainingTime = 0.0f;

	SetVisibility(ESlateVisibility::Collapsed); // 위젯을 화면에서 숨김
	BP_OnRespawned();
}

float USMPlayerDeathWidget::GetRemainingTime() const
{
	return RemainingTime;
}

void USMPlayerDeathWidget::NativeDestruct()
{
	bCountingDown = false;
	Super::NativeDestruct();
}

void USMPlayerDeathWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	
	if (!bCountingDown) return;
	RemainingTime -= DeltaTime;

	if (RemainingTime <= 0.0f)
	{
		RemainingTime = 0.0f;
		bCountingDown = false;
	}

	UpdateCountdownText();
}

void USMPlayerDeathWidget::UpdateCountdownText()
{
	if (!TextBlock_Countdown) return;
	
	const int32 Seconds = FMath::CeilToInt(FMath::Max(0.0f, RemainingTime));
	FText FinalText = FText::Format(CountdownFormatText, FText::AsNumber(Seconds));
	TextBlock_Countdown->SetText(FinalText);
}