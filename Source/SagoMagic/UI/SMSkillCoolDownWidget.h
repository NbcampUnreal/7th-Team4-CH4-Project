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
    // 퀵슬롯 변경 감지 → 감시 태그 갱신
    void RegisterMessageListener();
    void UnregisterMessageListener();
    void HandleQuickSlotUpdated(FGameplayTag Channel,
                                const FSMQuickSlotUpdatedMessage& Message);

    // ASC 쿨다운 GE 감지
    void BindASCDelegates();
    void UnbindASCDelegates();
    void OnEffectAdded(UAbilitySystemComponent* ASC,
                       const FGameplayEffectSpec& Spec,
                       FActiveGameplayEffectHandle Handle);
    void OnEffectRemoved(const FActiveGameplayEffect& Effect);

    // 현재 활성 스킬 태그 기반으로 쿨다운 태그 갱신
    void RefreshWatchedTag();
    // ASC에서 남은 시간 읽어 UI 갱신
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

    float CooldownDuration  = 0.f;
    float CooldownRemaining = 0.f;
    bool  bIsOnCooldown     = false;
};