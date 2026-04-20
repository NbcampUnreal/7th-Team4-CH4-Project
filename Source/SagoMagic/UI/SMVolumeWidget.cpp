#include "SMVolumeWidget.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Core/DataManager/SMSoundManager.h"

void USMVolumeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	USMSoundManager* SM = USMSoundManager::Get(this);
	if (!SM) return;
	
	if (SliderMaster)
	{
		SliderMaster->SetValue(SM->GetMasterVolume());
		SliderMaster->OnValueChanged.AddDynamic(this, &USMVolumeWidget::OnMasterVolumeChanged);
		UpdatePercentText(TextMaster, SM->GetMasterVolume());
	}
	if (SliderBGM)
	{
		SliderBGM->SetValue(SM->GetBGMVolume());
		SliderBGM->OnValueChanged.AddDynamic(this, &USMVolumeWidget::OnBGMVolumeChanged);
		UpdatePercentText(TextMaster, SM->GetBGMVolume());
	}
	if (SliderSFX)
	{
		SliderSFX->SetValue(SM->GetSFXVolume());
		SliderSFX->OnValueChanged.AddDynamic(this, &USMVolumeWidget::OnSFXVolumeChanged);
		UpdatePercentText(TextMaster, SM->GetSFXVolume());
	}
}

void USMVolumeWidget::OnMasterVolumeChanged(float Value)
{
	if (USMSoundManager* SM = USMSoundManager::Get(this))
		SM->SetMasterVolume(Value);
	UpdatePercentText(TextMaster, Value);
}

void USMVolumeWidget::OnBGMVolumeChanged(float Value)
{
	if (USMSoundManager* SM = USMSoundManager::Get(this))
		SM->SetBGMVolume(Value);
	UpdatePercentText(TextBGM, Value);
}

void USMVolumeWidget::OnSFXVolumeChanged(float Value)
{
	if (USMSoundManager* SM = USMSoundManager::Get(this))
		SM->SetSFXVolume(Value);
	UpdatePercentText(TextSFX, Value);
}

void USMVolumeWidget::UpdatePercentText(UTextBlock* TextBlock, float Value)
{
	if (!TextBlock) return;
	TextBlock->SetText(
		FText::FromString(
			FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value* 100.f))));
}
