#include "SMGameResultWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

bool USMGameResultWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (Button_Restart)
	{
		Button_Restart->OnClicked.AddDynamic(this, &ThisClass::OnRestartClicked);
	}

	if (Button_Exit)
	{
		Button_Exit->OnClicked.AddDynamic(this, &ThisClass::OnQuitClicked);
	}

	return true;
}

void USMGameResultWidget::NativeDestruct()
{
	if (Button_Restart) Button_Restart->OnClicked.RemoveAll(this);
	if (Button_Exit)    Button_Exit->OnClicked.RemoveAll(this);
	
	Super::NativeDestruct();
}

void USMGameResultWidget::ShowResult(bool bIsVictory)
{
	// 결과 텍스트 세팅
	if (TextBlock_ResultTitle)
	{
		TextBlock_ResultTitle->SetText(bIsVictory ? VictoryText : DefeatText);
	}
	// UI 입력 모드 및 마우스 커서 표시 세팅
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		if (Button_Restart)
		{
			InputMode.SetWidgetToFocus(Button_Restart->TakeWidget());
		}
		PC->SetInputMode(InputMode);
	}

	if (bIsVictory)
	{
		BP_OnVictory();
	}
	else
	{
		BP_OnDefeat();
	}
}

void USMGameResultWidget::OnRestartClicked()
{
	// 재시작 버튼 클릭 시 로비 맵으로 이동
	if (APlayerController* PC = GetOwningPlayer())
	{
		// 클라이언트 → 타이틀 또는 로비로 이동
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("L_Lobby")), true);
	}
}

void USMGameResultWidget::OnQuitClicked()
{
	// 종료 버튼 클릭 시 게임 완전 종료
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}