#include "SMHUDManager.h"
#include "UI/SMHUDManager.h"
#include "UI/SMPlayerHPBarWidget.h"
#include "AbilitySystemComponent.h"


void USMHUDManager::NativeConstruct()
{
	Super::NativeConstruct();
}

void USMHUDManager::InitializeHUD(UAbilitySystemComponent* InPlayerASC)
{
	if (!InPlayerASC) return;
	
	if (WBP_PlayerBar)
	{
		WBP_PlayerBar->InitializeWithASC(InPlayerASC);
	}
}

void USMHUDManager::RefreshHUD(UAbilitySystemComponent* InPlayerASC)
{
	InitializeHUD(InPlayerASC);
}
