#include "SMGameResultWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USMGameResultWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
	Super::NativeDestruct();
}

void USMGameResultWidget::ShowResult(bool bIsVictory)
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
		PC->SetInputMode(FInputModeUIOnly());
	}

	if (bIsVictory) BP_OnVictory();
	else BP_OnDefeat();
	
	RemainingTime = ReturnToLobbyDelay; // 카운트다운 초기화
 	
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
		ReturnToLobby();
		return;
	}
	RemainingTime -= 1.0f;
}

void USMGameResultWidget::ReturnToLobby()
{
	if (LobbyLevelName.IsNone()) return;
	// 로비 레벨로 자동 이동
	UGameplayStatics::OpenLevel(GetWorld(), LobbyLevelName, true);
}