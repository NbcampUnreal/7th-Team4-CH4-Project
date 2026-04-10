#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMGameplayMessages.h"
#include "SMHUDManager.generated.h"


class USMPlayerStatusWidget;
class UAbilitySystemComponent;
class USMGameResultWidget;
class USMPlayerDeathWidget;
/**
 * 인게임 HUD 루트 위젯 - 뷰포트에 올라가며 자식 위젯들 소유
 * 플레이어 캐릭터 찾아 ASC 확보 -> 자식 위젯에 넘겨줌
 */
UCLASS()
class SAGOMAGIC_API USMHUDManager : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** ASC를 받아 각 자식 위젯에 전달 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeHUD(UAbilitySystemComponent* InPlayerASC);
	/** 리스폰 등으로 새 폰이 생겼을 때 재연결 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHUD(UAbilitySystemComponent* InPlayerASC);
	/** 게임 결과 위젯 표시 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameResult(bool bIsVictory, float InReturnDelay);
	/** 플레이어 사망 위젯 표시 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowPlayerDeath(float RespawnTime);
	/** 플레이어 부활 → 사망 위젯 숨김 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HidePlayerDeath();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="HUD")
	TObjectPtr<USMPlayerStatusWidget> WBP_PlayerStatus;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD")
	TObjectPtr<USMGameResultWidget> WBP_GameResult;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "HUD")
	TObjectPtr<USMPlayerDeathWidget> WBP_PlayerDeath;
	
private:
	/** ASC를 안전하게 가져오기 위해 재시도 */
	void TryInitASC();
	
	/** 재시도 타이머를 관리할 타이머 핸들 */
	FTimerHandle ASC_InitTimerHandle;


	UPROPERTY()
	FGameplayMessageListenerHandle PlayerStatusListenerHandle;
	UPROPERTY()
	FGameplayMessageListenerHandle PlayerRespawnListenerHandle;
	UPROPERTY()
	FGameplayMessageListenerHandle GameResultListenerHandle;
};
