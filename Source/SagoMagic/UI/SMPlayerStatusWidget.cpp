#include "SMPlayerStatusWidget.h"
#include "SMPlayerHPBarWidget.h"
#include "SMPlayerGoldWidget.h"
#include "UI/Inventory/SMQuickSlotBarWidget.h"
#include "Inventory/Components/SMInventoryComponent.h"

void USMPlayerStatusWidget::InitializeStatus(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;
	
	if (WBP_PlayerBar) // HP바 위젯 바인딩 -> ASC 전달
	{
		WBP_PlayerBar->InitializeWithASC(InASC);
	}
	
	if (WBP_GoldDisplay)
	{
		WBP_GoldDisplay->InitializeWithASC(InASC);
	}
	
	if (WBP_QuickSlotBar)
	{
		if (APlayerController* PC = GetOwningPlayer())
		{
			if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
			{
				if (USMInventoryComponent* Inv = PS->FindComponentByClass<USMInventoryComponent>())
				{
					WBP_QuickSlotBar->InitializeQuickSlotBarWidget(Inv);
				}
			}
		}
	}
}
