#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "SMWaveTimeWidget.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 웨이브 상태, 인덱스, 남은 시간을 위젯으로 띄움
 */
UCLASS()
class SAGOMAGIC_API USMWaveTimeWidget : public UUserWidget
{
	GENERATED_BODY()
    
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    
	/** GMS 메시지 수신 함수 */
	void OnWaveMessageReceived(FGameplayTag Channel, const FWaveMsg& Message);
	/** 남은 시간에 맞춰 프로그레스바 비율을 갱신하는 함수 */
	void UpdateProgressBar();
	/** 남은 시간을 분:초 형태의 텍스트로 갱신하는 함수 */
	void UpdateTimeText();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_WaveTime;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_WaveIndex;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_TimeRemaining;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_PhaseLabel;

	UPROPERTY(EditDefaultsOnly, Category = "Wave UI|Settings")
	FText BuildPhaseLabel = FText::FromString(TEXT("정비 단계"));
	UPROPERTY(EditDefaultsOnly, Category = "Wave UI|Settings")
	FText CombatPhaseLabel = FText::FromString(TEXT("전투 단계"));
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Wave UI")
	void BP_OnPhaseChanged(EWaveUIState NewState);

private:
	FGameplayMessageListenerHandle WaveListenerHandle;

	EWaveUIState CurrentState;
	int32 CurrentWaveIndex = 0; 
	float LocalTimeRemaining = 0.f;
	float CurrentMaxTime = 0.f;
	
	int32 LastUpdatedSeconds = -1;
};