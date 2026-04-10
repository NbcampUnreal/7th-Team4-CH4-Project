#include "SMGameResultWidget.h"
#include "Components/TextBlock.h"

void USMGameResultWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	Super::NativeDestruct();
}

void USMGameResultWidget::ShowResult(bool bIsVictory, float InReturnDelay)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	
	// 결과 텍스트 세팅
	if (TextBlock_ResultTitle)
	{
		TextBlock_ResultTitle->SetText(bIsVictory ? VictoryText : DefeatText);
	}
	
	// UI 입력 모드 및 커서 표시
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}

	if (bIsVictory) BP_OnVictory();
	else BP_OnDefeat();
	
	RemainingTime = InReturnDelay; // 서버에서 전달받은 시간 셋팅
 	
	UpdateCountdown();
	
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CountdownTimerHandle, this, &ThisClass::UpdateCountdown, 1.0f, true);
	}
}

void USMGameResultWidget::UpdateCountdown()
{
	if (TextBlock_RestartTime)
	{
		int32 Seconds = FMath::Max(0, FMath::CeilToInt(RemainingTime));
		FText Display = FText::Format(CountdownFormat, FText::AsNumber(Seconds));
		TextBlock_RestartTime->SetText(Display);
	}
	
	if (RemainingTime <= 0.0f)
	{
		if (UWorld* World = GetWorld()) // 타이머 먼저 확실히 종료
		{
			World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		}
		return;
	}
	RemainingTime -= 1.0f;
}