#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "Containers/Queue.h"
#include "SMNotificationWidget.generated.h"

class UTextBlock;
class USMPlayerInventoryPanelWidget;
/**
 * 게임 내 알림 메시지를 화면에 표시하는 UI 위젯 클래스
 * 큐를 활용 -> 알림이 겹치지 않고 순차적으로 표시되도록 보장함
 */
UCLASS()
class SAGOMAGIC_API USMNotificationWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Notification")
    void SetListenChannel(const FGameplayTag& InListenChannel);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Notification;
    
    UPROPERTY(EditDefaultsOnly, Category = "Notification|Settings")
    float FadeInDuration = 0.4f;
    UPROPERTY(EditDefaultsOnly, Category = "Notification|Settings")
    float FadeOutDuration = 0.4f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification|Settings")
    FGameplayTag ListenChannel;

private:
    void RegisterNotificationListener();
    void UnregisterNotificationListener();
    /** GMS 콜백 함수 */
    void OnNotificationReceived(FGameplayTag Channel, const FNotificationMsg& Message);
    /** 큐에 대기 중인 다음 메시지가 있다면 화면에 표시 시작 함수 */
    void TryShowNext();
    /** 현재 위젯이 구독해야 할 채널 결정 */
    FGameplayTag ResolveListenChannel() const;
    
    FGameplayMessageListenerHandle NotificationListenerHandle;
    
    /** 알림 메시지를 담아두는 대기열 -> 큐 구조인 TQueue가 FIFO 처리에 효율적 */
    TQueue<FNotificationMsg> MessageQueue;
 
    /** 알림 UI의 현재 진행 상태를 정의하는 열거형 */
    enum class ENotificationPhase : uint8
    {
       Idle,
       FadeIn,
       Display,
       FadeOut,
    };
 
    ENotificationPhase CurrentPhase = ENotificationPhase::Idle;
    float PhaseElapsed = 0.0f;
    float CurrentDisplayDuration = 2.0f;
};
