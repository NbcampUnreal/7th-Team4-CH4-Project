// SMSkillCooldownWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"
#include "SMSkillCooldownWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;
class USMInventoryComponent;

UCLASS()
class SAGOMAGIC_API USMSkillCooldownWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void InitializeCooldownWidget(UAbilitySystemComponent* InASC,
                                  USMInventoryComponent* InInventoryComponent);

protected:
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ProgressBar_Cooldown;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TextBlock_Cooldown;

private:
    void RegisterMessageListener();
    void UnregisterMessageListener();
    void HandleQuickSlotUpdated(FGameplayTag Channel,
                                const FSMQuickSlotUpdatedMessage& Message);

    void BindASCDelegates();
    void UnbindASCDelegates();
    void OnEffectAdded(UAbilitySystemComponent* ASC,
                       const FGameplayEffectSpec& Spec,
                       FActiveGameplayEffectHandle Handle);
    void OnEffectRemoved(const FActiveGameplayEffect& Effect);

    // 인벤토리 캐시에서 FinalCooldown + CooldownTag 갱신
    void RefreshWatchedTag();
    // ASC에서 남은 시간만 읽음 (Duration은 인벤 캐시 사용)
    void RefreshCooldownState();
    void UpdateVisuals();

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> BoundASC;
    UPROPERTY()
    TObjectPtr<USMInventoryComponent> BoundInventory;

    FGameplayTag WatchedCooldownTag;

    FDelegateHandle EffectAddedHandle;
    FDelegateHandle EffectRemovedHandle;
    FGameplayMessageListenerHandle QuickSlotListenerHandle;

    // CooldownDuration은 인벤토리 FinalCooldown 캐시에서만 설정
    float CachedFinalCooldown = 0.f;
    float CooldownRemaining   = 0.f;
    bool  bIsOnCooldown       = false;
    
    // Tick 최적화용: 마지막으로 확인한 활성 스킬 태그 캐싱
    FGameplayTag LastKnownActiveSkillTag;
};