#include "UI/SMNotificationWidget.h"
#include "Components/TextBlock.h"
#include "GameplayTags/UI/SMUITag.h"

void USMNotificationWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    SetRenderOpacity(0.0f); // 위젯 디폴트값을 보이지 않게

    // 메시지 시스템 구독
    if (UGameplayMessageSubsystem::HasInstance(this))
    {
       UGameplayMessageSubsystem& MsgSys = UGameplayMessageSubsystem::Get(this);
       NotificationListenerHandle = MsgSys.RegisterListener<FNotificationMsg>(
          SMUITag::Event_Notification, this, &ThisClass::OnNotificationReceived);
    }
}

void USMNotificationWidget::NativeDestruct()
{
    if (NotificationListenerHandle.IsValid())
    {
       NotificationListenerHandle.Unregister();
    }
    Super::NativeDestruct();
}

void USMNotificationWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    if (CurrentPhase == ENotificationPhase::Idle)
    {
       return;
    }
    
    PhaseElapsed += DeltaTime; // 현재 페이즈가 시작된 이후 흐른 시간 누적
    
    switch (CurrentPhase) // 페이드 인 -> 대기 -> 페이드 아웃 로직 처리
    {
        case ENotificationPhase::FadeIn:
        {
            const float Alpha = (FadeInDuration > KINDA_SMALL_NUMBER)
                ? FMath::Clamp(PhaseElapsed / FadeInDuration, 0.0f, 1.0f) : 1.0f;
            SetRenderOpacity(Alpha);
                
            if (PhaseElapsed >= FadeInDuration)
            {
                SetRenderOpacity(1.0f);
                CurrentPhase = ENotificationPhase::Display;
                PhaseElapsed = 0.0f;
            }
            break;
        }

        case ENotificationPhase::Display:
        {
            if (PhaseElapsed >= CurrentDisplayDuration)
            {
                CurrentPhase = ENotificationPhase::FadeOut;
                PhaseElapsed = 0.0f;
            }
            break;
        }

        case ENotificationPhase::FadeOut:
        {
            const float Alpha = (FadeOutDuration > KINDA_SMALL_NUMBER)
                ? FMath::Clamp(1.0f - (PhaseElapsed / FadeOutDuration), 0.0f, 1.0f) : 0.0f;
            SetRenderOpacity(Alpha);
                
            if (PhaseElapsed >= FadeOutDuration)
            {
                SetRenderOpacity(0.0f);
                CurrentPhase = ENotificationPhase::Idle;
                PhaseElapsed = 0.0f;
                
                TryShowNext();
            }
            break;
        }
    }
}

void USMNotificationWidget::OnNotificationReceived(FGameplayTag Channel, const FNotificationMsg& Message)
{
    // 새로운 알림이 들어오면 큐의 맨 뒤에 추가
    MessageQueue.Enqueue(Message);

    // 알림창 Idle 상태라면 즉시 큐에서 꺼내어 보여주기 시작
    if (CurrentPhase == ENotificationPhase::Idle)
    {
       TryShowNext();
    }
}

void USMNotificationWidget::TryShowNext()
{
    if (MessageQueue.IsEmpty())
    {
       return;
    }
    
    FNotificationMsg NextMsg; // 큐의 맨 앞에서 메시지를 하나 꺼냄
    if (MessageQueue.Dequeue(NextMsg))
    {
        if (IsValid(TextBlock_Notification))
        {
            TextBlock_Notification->SetText(NextMsg.Message);
        }
        
        CurrentDisplayDuration = NextMsg.DisplayDuration;
        CurrentPhase = ENotificationPhase::FadeIn;
        PhaseElapsed = 0.0f;
        
        SetRenderOpacity(0.0f);
    }
}