#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPlayerDeathWidget.generated.h"

class UTextBlock;
/**
 * 플레이어 사망 위젯
 * 부활 카운트다운 표시, 부활 시 자동 숨김 처리
 */
UCLASS()
class SAGOMAGIC_API USMPlayerDeathWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 사망 위젯을 화면에 띄우고 카운트다운 시작 */
	UFUNCTION(BlueprintCallable)
	void ShowDeathWidget(float InRespawnTime);
	/** 부활 완료 → 위젯 숨김 */
	UFUNCTION(BlueprintCallable)
	void HideDeathWidget();
	/** 현재 남은 부활 시간 */
	UFUNCTION(BlueprintPure)
	float GetRemainingTime() const;

protected:
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

	/** 부활 카운트다운 텍스트 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Countdown;
	/** 사망 문구 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_DeathMessage;
	
	/** 부활 완료 시 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Death")
	void BP_OnRespawned();
	/** 사망 시 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Player Death")
	void BP_OnDied();
	
	/** 메인 사망 문구 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Death")
	FText DeathMessageText = FText::FromString(TEXT("You Died"));
	/** 카운트다운 문구 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Death")
	FText CountdownFormatText = FText::FromString(TEXT("Respawning in {0}..."));

private:
	/** 남은 부활 시간 */
	float RemainingTime = 0.0f;
	/** 카운트다운 진행 여부 */
	bool bCountingDown = false;
	
	void UpdateCountdownText();
};
