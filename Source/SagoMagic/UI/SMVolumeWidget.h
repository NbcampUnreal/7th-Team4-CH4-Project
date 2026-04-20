#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMVolumeWidget.generated.h"

class UTextBlock;
class USlider;
/**
 * Sound Volume Widget
 */
UCLASS()
class SAGOMAGIC_API USMVolumeWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SliderMaster;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SliderBGM;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SliderSFX;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextMaster;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBGM;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextSFX;
	
private:
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);
	
	UFUNCTION()
	void OnBGMVolumeChanged(float Value);
	
	UFUNCTION()
	void OnSFXVolumeChanged(float Value);
	
	void UpdatePercentText(UTextBlock* TextBlock, float Value);
};
