// USMPlayerNicknameWidget.cpp


#include "SMPlayerNicknameWidget.h"
#include "Components/TextBlock.h"

void USMPlayerNicknameWidget::SetPlayerNickName(const FString& InNickName)
{
	if (IsValid(NicknameTextBlock) == false) return;
	NicknameTextBlock->SetText(FText::FromString(InNickName));
}
