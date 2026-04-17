// SMSkillCooldownWidget.cpp
#include "UI/SMSkillCooldownWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameplayEffect.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/Message/SMMessageTag.h"
#include "Inventory/Components/SMInventoryComponent.h"

void USMSkillCooldownWidget::InitializeCooldownWidget(UAbilitySystemComponent* InASC,
                                                       USMInventoryComponent* InInventoryComponent)
{
    UnbindASCDelegates();
    UnregisterMessageListener();

    BoundASC       = InASC;
    BoundInventory = InInventoryComponent;

    if (BoundASC && BoundInventory)
    {
        BindASCDelegates();
        RegisterMessageListener();
        RefreshWatchedTag(); // 현재 슬롯 스킬 태그 즉시 반영
    }
}

void USMSkillCooldownWidget::NativeDestruct()
{
    UnbindASCDelegates();
    UnregisterMessageListener();
    Super::NativeDestruct();
}

void USMSkillCooldownWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // [추가] 클라이언트 복제 지연(Race Condition)을 방어하기 위한 태그 동기화 로직
    if (BoundInventory)
    {
        FGameplayTag CurrentActiveTag = BoundInventory->GetActiveSkillTag();
        if (CurrentActiveTag.IsValid())
        {
            FString CooldownTagStr = CurrentActiveTag.ToString().Replace(TEXT("Ability."), TEXT("Cooldown."));
            // ErrorIfNotFound를 임시로 무시하고 태그를 찾음
            FGameplayTag ExpectedCooldownTag = FGameplayTag::RequestGameplayTag(FName(*CooldownTagStr), false);

            // UI가 감시하는 태그와 실제 활성화된 태그가 다르면 즉시 갱신
            if (WatchedCooldownTag != ExpectedCooldownTag)
            {
                RefreshWatchedTag();
            }
        }
    }

    // 쿨다운 중일 때만 시간 갱신
    if (bIsOnCooldown)
    {
        RefreshCooldownState();
    }
}

// ─── 퀵슬롯 메시지 ───────────────────────────────────────────

void USMSkillCooldownWidget::RegisterMessageListener()
{
    QuickSlotListenerHandle =
        UGameplayMessageSubsystem::Get(this)
            .RegisterListener<FSMQuickSlotUpdatedMessage>(
                SMMessageTag::Inventory_QuickSlotUpdated,
                this,
                &ThisClass::HandleQuickSlotUpdated);
}

void USMSkillCooldownWidget::UnregisterMessageListener()
{
    if (QuickSlotListenerHandle.IsValid())
        QuickSlotListenerHandle.Unregister();
}

void USMSkillCooldownWidget::HandleQuickSlotUpdated(FGameplayTag /*Channel*/,
                                                     const FSMQuickSlotUpdatedMessage& Message)
{
    // 내 플레이어 상태인지 확인
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
        {
            if (Message.GetOwningPlayerState() != PS) return;
        }
    }

    RefreshWatchedTag();
}

// ─── 감시 태그 갱신 ──────────────────────────────────────────

void USMSkillCooldownWidget::RefreshWatchedTag()
{
    if (!BoundInventory) return;

    // 현재 활성 슬롯의 스킬 태그 ("Ability.Skill.Projectile" 등)
    FGameplayTag ActiveSkillTag = BoundInventory->GetActiveSkillTag();

    if (!ActiveSkillTag.IsValid())
    {
        WatchedCooldownTag = FGameplayTag();
        bIsOnCooldown      = false;
        UpdateVisuals();
        return;
    }

    // "Ability.Skill.Xxx" → "Cooldown.Skill.Xxx"
    FString CooldownTagStr =
        ActiveSkillTag.ToString().Replace(TEXT("Ability."), TEXT("Cooldown."));
    WatchedCooldownTag = FGameplayTag::RequestGameplayTag(FName(*CooldownTagStr), false);

    RefreshCooldownState();
}

// ─── ASC 델리게이트 ──────────────────────────────────────────

void USMSkillCooldownWidget::BindASCDelegates()
{
    if (!BoundASC) return;

    EffectAddedHandle = BoundASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(
        this, &ThisClass::OnEffectAdded);

    EffectRemovedHandle = BoundASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(
        this, &ThisClass::OnEffectRemoved);
}

void USMSkillCooldownWidget::UnbindASCDelegates()
{
    if (!BoundASC) return;

    if (EffectAddedHandle.IsValid())
    {
        BoundASC->OnActiveGameplayEffectAddedDelegateToSelf.Remove(EffectAddedHandle);
        EffectAddedHandle.Reset();
    }
    if (EffectRemovedHandle.IsValid())
    {
        BoundASC->OnAnyGameplayEffectRemovedDelegate().Remove(EffectRemovedHandle);
        EffectRemovedHandle.Reset();
    }

    BoundASC = nullptr;
}

// ─── ASC 델리게이트 ──────────────────────────────────────────

void USMSkillCooldownWidget::OnEffectAdded(UAbilitySystemComponent* /*ASC*/,
                                            const FGameplayEffectSpec& Spec,
                                            FActiveGameplayEffectHandle /*Handle*/)
{
    if (!WatchedCooldownTag.IsValid()) return;

    // [수정 사항] DynamicGrantedTags뿐만 아니라 GE(블루프린트)에 기본 설정된 
    // GrantedTags까지 모두 합쳐서 가져옵니다.
    FGameplayTagContainer AllGrantedTags;
    Spec.GetAllGrantedTags(AllGrantedTags);

    // 이제 이 GE가 우리가 감시하는 쿨타임 태그를 가지고 있는지 정확히 판별할 수 있습니다.
    if (AllGrantedTags.HasTag(WatchedCooldownTag))
    {
        RefreshCooldownState();
    }
}

void USMSkillCooldownWidget::OnEffectRemoved(const FActiveGameplayEffect& Effect)
{
    if (!WatchedCooldownTag.IsValid()) return;

    // [수정 사항] 제거될 때도 마찬가지로 전체 부여 태그(AllGrantedTags)를 조회하여 검사합니다.
    FGameplayTagContainer AllGrantedTags;
    Effect.Spec.GetAllGrantedTags(AllGrantedTags);

    // 감시 중인 쿨타임 태그가 제거된 것이 맞다면 UI를 초기화합니다.
    if (AllGrantedTags.HasTag(WatchedCooldownTag))
    {
        bIsOnCooldown      = false;
        CooldownRemaining  = 0.f;
        CooldownDuration   = 0.f;
        UpdateVisuals();
    }
}

// ─── 쿨다운 상태 읽기 ────────────────────────────────────────

void USMSkillCooldownWidget::RefreshCooldownState()
{
    if (!BoundASC || !WatchedCooldownTag.IsValid())
    {
        bIsOnCooldown = false;
        UpdateVisuals();
        return;
    }

    FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(WatchedCooldownTag));

    TArray<float> Remaining = BoundASC->GetActiveEffectsTimeRemaining(Query);
    TArray<float> Durations = BoundASC->GetActiveEffectsDuration(Query);

    if (Remaining.IsEmpty() || Remaining[0] <= 0.f)
    {
        bIsOnCooldown     = false;
        CooldownRemaining = 0.f;
        CooldownDuration  = 0.f;
    }
    else
    {
        bIsOnCooldown     = true;
        CooldownRemaining = Remaining[0];
        CooldownDuration  = FMath::Max(KINDA_SMALL_NUMBER, Durations[0]);
    }

    UpdateVisuals();
}

// ─── UI 반영 ─────────────────────────────────────────────────

void USMSkillCooldownWidget::UpdateVisuals()
{
    const float Percent = bIsOnCooldown
        ? FMath::Clamp(CooldownRemaining / CooldownDuration, 0.f, 1.f)
        : 0.f;

    if (ProgressBar_Cooldown)
        ProgressBar_Cooldown->SetPercent(Percent);

    if (TextBlock_Cooldown)
    {
        if (bIsOnCooldown)
        {
            TextBlock_Cooldown->SetText(
                FText::FromString(FString::Printf(TEXT("%.1f"), CooldownRemaining)));
            TextBlock_Cooldown->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            TextBlock_Cooldown->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // 쿨다운 없으면 위젯 전체 숨김
    SetVisibility(bIsOnCooldown
        ? ESlateVisibility::HitTestInvisible
        : ESlateVisibility::Collapsed);
}