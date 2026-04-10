#include "SMBaseCampHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayTags/UI/SMUITag.h"

void USMBaseCampHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Listener 등록
	if (UGameplayMessageSubsystem::HasInstance(this))
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
        
		BaseCampListenerHandle = MessageSystem.RegisterListener(SMUITag::Event_BaseCamp, this,
			&ThisClass::OnBaseCampMessageReceived);
	}
}

void USMBaseCampHPBarWidget::NativeDestruct()
{
	if (BaseCampListenerHandle.IsValid())
	{
		BaseCampListenerHandle.Unregister();
	}
    
	Super::NativeDestruct();
}

void USMBaseCampHPBarWidget::OnBaseCampMessageReceived(FGameplayTag Channel, const FBaseCampMsg& Message)
{
	// 방송 수신하면 프로그레스 바 갱신
	if (ProgressBar_BaseCampHP)
	{
		ProgressBar_BaseCampHP->SetPercent(Message.BaseCampHP);
	}

	if (TextBlock_BaseCampHP)
	{
		int32 Current = FMath::Max(0, FMath::CeilToInt(Message.CurrentHP));
		int32 Max = FMath::Max(1, FMath::CeilToInt(Message.MaxHP));
		
		FText HPText = FText::Format(FText::FromString(TEXT("{0} / {1}")), Current, Max);
		TextBlock_BaseCampHP->SetText(HPText);
	}
}