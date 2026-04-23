// SMTitleWidget.cpp


#include "SMTitleWidget.h"
#include "Character/SMTitlePlayerController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Core/SMGameInstance.h"

void USMTitleWidget::MenuSetup()
{
	if (IsValid(IPInputBox) == true)
	{
		IPInputBox->SetText(FText::FromString(TEXT("127.0.0.1:17777")));
	}
	if (IsValid(NicknameInputBox) == true)
	{
		NicknameInputBox->SetHintText(FText::FromString(TEXT("닉네임을 입력하세요.")));
		NicknameInputBox->SetToolTipText(
			FText::FromString(FString::Printf(TEXT("공백 포함 최대 %d글자"), NickNameLengthLimit)));
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

	if (USMGameInstance* GI = GetGameInstance<USMGameInstance>())
	{
		FString InputNickname;
		FString TrimNickname;
		if (IsValid(NicknameInputBox) == true)
		{
			InputNickname = NicknameInputBox->GetText().ToString().TrimStartAndEnd();
			TrimNickname = InputNickname.Left(NickNameLengthLimit);
		}
		if (TrimNickname.IsEmpty() == true)
		{
			TrimNickname = TEXT("Player");
		}

		GI->SetPendingNickname(TrimNickname);
	}

	ASMTitlePlayerController* PC = GetSMTitlePlayerController();
	if (PC)
	{
		PC->SetPendingServerAddress(ServerAddress);
	}
}

ASMTitlePlayerController* USMTitleWidget::GetSMTitlePlayerController() const
{
	return Cast<ASMTitlePlayerController>(GetOwningPlayer());
}
