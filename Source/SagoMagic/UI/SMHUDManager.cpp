#include "SMHUDManager.h"
#include "UI/SMPlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"


void USMHUDManager::NativeConstruct()
{
	Super::NativeConstruct();
	TryInitASC();
}

void USMHUDManager::TryInitASC()
{
	/** HUD가 플레이어 폰을 찾아 ASC 연동 시도 */
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->PlayerState) // 폰이 죽어도 살아있어서 더 안정!
		{
			if (APawn* OwningPawn = PC->GetPawn())
			{
				// 캐릭터가 IAbilitySystemInterface를 상속받았는지 확인 후 ASC 가져옴
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwningPawn))
				{
					UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
					if (ASC && ASC->GetAvatarActor() == OwningPawn)
					{
						InitializeHUD(ASC);
						// 연동 성공 -> 타이머 해제
						GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
						return;
					}
				}
			}
		}
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(ASC_InitTimerHandle, this, &USMHUDManager::TryInitASC, 0.1f, false);
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
