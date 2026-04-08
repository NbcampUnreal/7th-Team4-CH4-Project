#include "SMHUDManager.h"
#include "UI/SMPlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "TimerManager.h"
#include "UI/SMGameResultWidget.h"
#include "UI/SMPlayerDeathWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void USMHUDManager::NativeConstruct()
{
	Super::NativeConstruct();
	TryInitASC();
	
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	// 사망 리스너
	PlayerStatusListenerHandle = MessageSubsystem.RegisterListener<FPlayerStatusMsg>(
		FGameplayTag::RequestGameplayTag(TEXT("UI.Event.Player.Dead")),
		this,
		&USMHUDManager::OnPlayerStatusMessageReceived
	);
	// 부활 리스너
	PlayerRespawnListenerHandle = MessageSubsystem.RegisterListener<FPlayerStatusMsg>(
		FGameplayTag::RequestGameplayTag(TEXT("UI.Event.Player.Respawn")),
		this,
		&USMHUDManager::OnPlayerRespawnMessageReceived
	);
	// 게임 결과 리스너
	GameResultListenerHandle = MessageSubsystem.RegisterListener<FResultMsg>(
	FGameplayTag::RequestGameplayTag(TEXT("UI.Event.Result")),
	this,
	&USMHUDManager::OnGameResultMessageReceived
);
}

void USMHUDManager::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
	}
	
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		UGameplayMessageSubsystem& Sub = UGameplayMessageSubsystem::Get(this);
		Sub.UnregisterListener(PlayerStatusListenerHandle);
		Sub.UnregisterListener(PlayerRespawnListenerHandle);
		Sub.UnregisterListener(GameResultListenerHandle);
	}
	
	Super::NativeDestruct();
}

void USMHUDManager::TryInitASC()
{
	/** HUD가 플레이어 폰을 찾아 ASC 연동 시도 */
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>()) // 폰이 죽어도 살아있어서 더 안정!
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

void USMHUDManager::OnPlayerStatusMessageReceived(FGameplayTag Channel, const FPlayerStatusMsg& Payload)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly());
		PC->SetShowMouseCursor(true);
	}

	// RespawnTime 값을 사망 위젯으로 넘겨줌
	ShowPlayerDeath(Payload.RespawnTime);
}

void USMHUDManager::OnPlayerRespawnMessageReceived(FGameplayTag Channel, const FPlayerStatusMsg& Payload)
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->SetShowMouseCursor(true);
	}
	HidePlayerDeath();
}

void USMHUDManager::OnGameResultMessageReceived(FGameplayTag Channel, const FResultMsg& Payload)
{
	ShowGameResult(Payload.bIsVictory);
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

void USMHUDManager::ShowGameResult(bool bIsVictory)
{
	if (WBP_GameResult)
	{
		WBP_GameResult->SetVisibility(ESlateVisibility::Visible);
		WBP_GameResult->ShowResult(bIsVictory);
	}
}

void USMHUDManager::ShowPlayerDeath(float RespawnTime)
{
	if (WBP_PlayerDeath)
	{
		WBP_PlayerDeath->ShowDeathWidget(RespawnTime);
	}
}

void USMHUDManager::HidePlayerDeath()
{
	if (WBP_PlayerDeath)
	{
		WBP_PlayerDeath->HideDeathWidget();
	}
}
