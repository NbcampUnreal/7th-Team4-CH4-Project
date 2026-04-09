#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMGameResultWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 게임 결과 위젯
 * GameMode나 ResultState 쪽에서 호출해서 표시하기
 */
UCLASS()
class SAGOMAGIC_API USMGameResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 결과 위젯 초기화  */
	UFUNCTION(BlueprintCallable)
	void ShowResult(bool bIsVictory);

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/** 승리,패배 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_ResultTitle;
	/** 다시 시작 버튼 (로비로 이동) */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Restart;
	/** 게임 종료 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Exit;
	
	/** 승리 시 & 패배 시 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Result")
	void BP_OnVictory();
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Result")
	void BP_OnDefeat();
	
	/** 에디터에서 지정할 승리 텍스트 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result|Settings")
	FText VictoryText = FText::FromString(TEXT("게임 클리어!"));
	/** 에디터에서 지정할 패배 텍스트 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result|Settings")
	FText DefeatText = FText::FromString(TEXT("방어 실패.."));
	/** 재시작 시 이동할 로비 맵 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result|Settings")
	FName LobbyLevelName = FName(TEXT("L_Lobby"));

private:
	UFUNCTION()
	void OnRestartClicked();
	UFUNCTION()
	void OnQuitClicked();
};
