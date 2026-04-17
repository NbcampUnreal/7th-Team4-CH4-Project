#include "UI/SMEditModeWidget.h"
#include "GameplayTags/UI/SMUITag.h"

void USMEditModeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MsgSys = UGameplayMessageSubsystem::Get(this);
	EditModeListenerHandle = MsgSys.RegisterListener<FEditModeMsg>(
		SMUITag::Event_EditMode, this, &ThisClass::OnEditModeMessage);

	SetVisibility(ESlateVisibility::Collapsed);
}

void USMEditModeWidget::NativeDestruct()
{
	if (EditModeListenerHandle.IsValid())
	{
		EditModeListenerHandle.Unregister();
	}
	Super::NativeDestruct();
}

void USMEditModeWidget::OnEditModeMessage(FGameplayTag Channel, const FEditModeMsg& Message)
{
	if (!Message.bIsActive)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		BP_OnModeActiveChanged(false);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BP_OnModeActiveChanged(true);
}