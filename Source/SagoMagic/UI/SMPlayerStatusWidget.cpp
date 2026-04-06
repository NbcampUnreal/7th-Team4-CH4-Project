#include "SMPlayerStatusWidget.h"
#include "SMPlayerHPBarWidget.h"

void USMPlayerStatusWidget::InitializeStatus(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;
	
	if (WBP_PlayerBar) // HP바 위젯 바인딩 -> ASC 전달
	{
		WBP_PlayerBar->InitializeWithASC(InASC);
	}
	
	/*if (WBP_GoldDisplay)
	{
		WBP_GoldDisplay->InitializeWithASC(InASC);
	}*/
}
