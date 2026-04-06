#include "SMHUDManager.h"
#include "UI/SMPlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

void USMHUDManager::NativeConstruct()
{
	Super::NativeConstruct();

	/** HUD가 플레이어 폰을 찾아 ASC 연동 시도 */
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* OwningPawn = PC->GetPawn())
		{
			// 캐릭터가 IAbilitySystemInterface를 상속받았는지 확인 후 ASC 가져옴
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwningPawn))
			{
				UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
				if (ASC)
				{
					InitializeHUD(ASC);
				}
			}
		}
	}
}

void USMHUDManager::InitializeHUD(UAbilitySystemComponent* InPlayerASC)
{
	if (!InPlayerASC) return;
	
	if (WBP_PlayerStatus) // 자식 위젯으로 데이터를 넘겨줌
	{
		WBP_PlayerStatus->InitializeStatus(InPlayerASC);
	}
}

void USMHUDManager::RefreshHUD(UAbilitySystemComponent* InPlayerASC)
{
	InitializeHUD(InPlayerASC);
}
