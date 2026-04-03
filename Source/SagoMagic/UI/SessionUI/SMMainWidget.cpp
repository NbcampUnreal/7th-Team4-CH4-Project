// SMMainWidget.cpp


#include "SMMainWidget.h"

#include "Character/SMPlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Core/SessionSubsystem/SMSessionSubsystem.h"

void USMMainWidget::MenuSetup()
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
		IPInputBox->SetText(FText::FromString(TEXT("127.0.0.1:7777")));
	}
}

bool USMMainWidget::Initialize()
{
	if (Super::Initialize() == false) return false;

	if (IsValid(HostButton) == true)
	{
		HostButton->OnClicked.AddDynamic(
			this, &ThisClass::OnHostButtonClicked);
	}

	return true;
}

void USMMainWidget::NativeDestruct()
{
	TearDown();
	Super::NativeDestruct();
}

void USMMainWidget::OnHostButtonClicked()
{
	HostButton->SetIsEnabled(false);

	FString ServerAddress = TEXT("127.0.0.1:7777");
	if (IsValid(IPInputBox) == true)
	{
		FString InputText = IPInputBox->GetText().ToString();
		if (!InputText.IsEmpty())
		{
			ServerAddress = InputText;
		}
	}

	ASMPlayerController* PC = GetSMPlayerController();
	if (PC)
	{
		PC->SetPendingServerAddress(ServerAddress);
	}

	if (SessionSubsystem)
	{
		SessionSubsystem->CreateSession(4);
	}
}

void USMMainWidget::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful == false)
	{
		HostButton->SetIsEnabled(true);
	}
}

void USMMainWidget::TearDown()
{
	if (SessionSubsystem)
	{
		SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(
			this, &ThisClass::OnCreateSessionComplete);
	}
}

ASMPlayerController* USMMainWidget::GetSMPlayerController() const
{
	return Cast<ASMPlayerController>(GetOwningPlayer());
}
