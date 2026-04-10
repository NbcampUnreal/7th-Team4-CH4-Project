#include "SMBaseCampHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayTags/UI/SMUITag.h"
#include "EngineUtils.h"
#include "Building/SMBaseCampActor.h"
#include "GAS/AttributeSets/SMBaseCampAttributeSet.h"
#include "AbilitySystemComponent.h"

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
	if (!TryInitializeFromBaseCamp())
	{
		GetWorld()->GetTimerManager().SetTimer(
			InitRetryTimerHandle,
			this,
			&USMBaseCampHPBarWidget::RetryInitialize,
			0.3f,
			true);
	}
}

void USMBaseCampHPBarWidget::NativeDestruct()
{
	if (BaseCampListenerHandle.IsValid())
	{
		BaseCampListenerHandle.Unregister();
	}
	GetWorld()->GetTimerManager().ClearTimer(InitRetryTimerHandle);
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

bool USMBaseCampHPBarWidget::TryInitializeFromBaseCamp()
{
	UWorld* World = GetWorld();
	if (!World) return false;

	for (TActorIterator<ASMBaseCampActor> It(World); It; ++It)
	{
		ASMBaseCampActor* BaseCamp = *It;
		if (!IsValid(BaseCamp)) continue;

		UAbilitySystemComponent* ASC = BaseCamp->GetAbilitySystemComponent();
		if (!ASC) continue;

		float MaxHP = ASC->GetNumericAttribute(
			USMBaseCampAttributeSet::GetMaxHealthAttribute());
		
		if (MaxHP <= 0.f) continue;

		float CurrentHP = ASC->GetNumericAttribute(
			USMBaseCampAttributeSet::GetHealthAttribute());

		FBaseCampMsg Msg;
		Msg.EventType  = EBaseCampEvent::Attacked;
		Msg.BaseCampHP = CurrentHP / MaxHP;
		Msg.CurrentHP  = CurrentHP;
		Msg.MaxHP      = MaxHP;
		OnBaseCampMessageReceived(SMUITag::Event_BaseCamp, Msg);
		return true;
	}
	return false;
}

void USMBaseCampHPBarWidget::RetryInitialize()
{
	if (TryInitializeFromBaseCamp())
	{
		GetWorld()->GetTimerManager().ClearTimer(InitRetryTimerHandle);
	}
}