#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMGameResultWidget.generated.h"

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
	void ShowResult(bool bIsVictory, float InReturnDelay);

protected:
	virtual void NativeDestruct() override;

	/** 승리,패배 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_ResultTitle;
	/** 다시시작 카운트다운 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_RestartTime;
	
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
	
	/** 카운트다운 표시 포맷 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result|Settings")
	FText CountdownFormat = FText::FromString(TEXT("{0}초 후 로비로 이동합니다..."));
	
private:
	/** 1초마다 실행되어 텍스트를 갱신할 함수 */
	UFUNCTION()
	void UpdateCountdown();

	/** 남은 시간 */
	float RemainingTime = 0.0f;
	/** 카운트다운용 타이머 핸들 */
	FTimerHandle CountdownTimerHandle;
};
