#include "UI/SMWaveTimeWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayTags/UI/SMUITag.h"

void USMWaveTimeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MsgSys = UGameplayMessageSubsystem::Get(this);
	// GMS 리스너 등록
	WaveListenerHandle = MsgSys.RegisterListener<FWaveMsg>(SMUITag::Event_Wave, this,
	                                                       &ThisClass::OnWaveMessageReceived);

	UpdateProgressBar();
	UpdateTimeText();
}

void USMWaveTimeWidget::NativeDestruct()
{
	if (WaveListenerHandle.IsValid())
	{
		WaveListenerHandle.Unregister();
	}
	Super::NativeDestruct();
}

void USMWaveTimeWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	if (LocalTimeRemaining > 0.f)
	{
		LocalTimeRemaining = FMath::Max(0.f, LocalTimeRemaining - DeltaTime);

		UpdateProgressBar();
		UpdateTimeText();
	}
}

void USMWaveTimeWidget::OnWaveMessageReceived(FGameplayTag Channel, const FWaveMsg& Message)
{
	const bool bPhaseChanged = (Message.State != CurrentState);

	CurrentState = Message.State;
	CurrentMaxTime = Message.MaxTime;
	
	if (bPhaseChanged || FMath::Abs(LocalTimeRemaining - Message.TimeRemaining) > 1.5f)
	{
		LocalTimeRemaining = Message.TimeRemaining;
	}

	if (CurrentWaveIndex != Message.WaveIndex)
	{
		CurrentWaveIndex = Message.WaveIndex;
		if (TextBlock_WaveIndex)
		{
			FText WaveText = FText::Format(FText::FromString(TEXT("Wave {0}")), FText::AsNumber(CurrentWaveIndex));
			TextBlock_WaveIndex->SetText(WaveText);
		}
	}

	if (bPhaseChanged)
	{
		if (TextBlock_PhaseLabel)
		{
			FText PhaseText = (CurrentState == EWaveUIState::Inprogress) ? CombatPhaseLabel : BuildPhaseLabel;
			TextBlock_PhaseLabel->SetText(PhaseText);
		}

		BP_OnPhaseChanged(CurrentState);
	}
	UpdateProgressBar();
	UpdateTimeText();
}

void USMWaveTimeWidget::UpdateProgressBar()
{
	if (ProgressBar_WaveTime)
	{
		const float Ratio = (CurrentMaxTime > 0.f) ? FMath::Clamp(LocalTimeRemaining / CurrentMaxTime, 0.f, 1.f) : 0.f;
		ProgressBar_WaveTime->SetPercent(Ratio);
	}
}

void USMWaveTimeWidget::UpdateTimeText()
{
	if (!TextBlock_TimeRemaining) return;

	int32 CurrentSeconds = FMath::CeilToInt(LocalTimeRemaining);

	if (CurrentSeconds != LastUpdatedSeconds)
	{
		LastUpdatedSeconds = CurrentSeconds;

		int32 Minutes = CurrentSeconds / 60;
		int32 Seconds = CurrentSeconds % 60;

		FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TextBlock_TimeRemaining->SetText(FText::FromString(TimeString));
	}
}
