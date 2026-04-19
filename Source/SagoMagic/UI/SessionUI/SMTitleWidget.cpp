// SMTitleWidget.cpp


#include "SMTitleWidget.h"
#include "Character/SMTitlePlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void USMTitleWidget::MenuSetup()
{
	if (IsValid(IPInputBox) == true)
	{
		IPInputBox->SetText(FText::FromString(TEXT("127.0.0.1:17777")));
	}
	
	// 안내 텍스트 초기 상태 숨김
	if (IsValid(LobbyFullText) == true)
	{
		LobbyFullText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USMTitleWidget::ShowLobbyFullMessage()
{
	if (IsValid(LobbyFullText) == false) return;

	LobbyFullText->SetVisibility(ESlateVisibility::Visible);

	// 3초 후 자동 숨김 — BindUObject로 안전하게 바인딩
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &USMTitleWidget::HideLobbyFullText);
	GetWorld()->GetTimerManager().SetTimer(LobbyFullTextTimer, TimerDel, 3.0f, false);
}

bool USMTitleWidget::Initialize()
{
	if (Super::Initialize() == false) return false;

	if (IsValid(HostButton) == true)
	{
		HostButton->OnClicked.AddDynamic(
			this, &ThisClass::OnHostButtonClicked);
	}

	return true;
}

void USMTitleWidget::NativeDestruct()
{
	// 위젯 소멸 시 타이머 정리 (댕글링 포인터 방지)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LobbyFullTextTimer);
	}
	
	Super::NativeDestruct();
}

void USMTitleWidget::OnHostButtonClicked()
{
	FString ServerAddress = TEXT("127.0.0.1:17777");
	if (IsValid(IPInputBox) == true)
	{
		FString InputText = IPInputBox->GetText().ToString();
		if (!InputText.IsEmpty())
		{
			ServerAddress = InputText;
		}
	}

	ASMTitlePlayerController* PC = GetSMTitlePlayerController();
	if (PC)
	{
		PC->TravelToCheck(ServerAddress);
	}
}

void USMTitleWidget::HideLobbyFullText()
{
	if (IsValid(LobbyFullText) == true)
	{
		LobbyFullText->SetVisibility(ESlateVisibility::Hidden);
	}
}

ASMTitlePlayerController* USMTitleWidget::GetSMTitlePlayerController() const
{
	return Cast<ASMTitlePlayerController>(GetOwningPlayer());
}
