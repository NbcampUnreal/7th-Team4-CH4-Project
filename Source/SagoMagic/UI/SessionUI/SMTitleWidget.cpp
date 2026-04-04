// SMTitleWidget.cpp


#include "SMTitleWidget.h"
#include "Character/SMTitlePlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Core/SessionSubsystem/SMSessionSubsystem.h"

void USMTitleWidget::MenuSetup()
{
	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI) == true)
	{
		SessionSubsystem = GI->GetSubsystem<USMSessionSubsystem>();
		if (SessionSubsystem)
		{
			SessionSubsystem->OnCreateSessionComplete.AddDynamic(
				this, &ThisClass::OnCreateSessionComplete);
		}
	}

	if (IsValid(IPInputBox) == true)
	{
		IPInputBox->SetText(FText::FromString(TEXT("127.0.0.1:17777")));
	}
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
	TearDown();
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
		PC->SetPendingServerAddress(ServerAddress);
	}

	if (SessionSubsystem)
	{
		SessionSubsystem->CreateSession(4);
	}
}

void USMTitleWidget::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		HostButton->SetIsEnabled(true);
	}
}

void USMTitleWidget::TearDown()
{
	if (SessionSubsystem)
	{
		SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(
			this, &ThisClass::OnCreateSessionComplete);
	}
}

ASMTitlePlayerController* USMTitleWidget::GetSMTitlePlayerController() const
{
	return Cast<ASMTitlePlayerController>(GetOwningPlayer());
}
