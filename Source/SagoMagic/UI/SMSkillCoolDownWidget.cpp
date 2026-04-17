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
#include "Inventory/Core/SMSkillRuntimeTypes.h"

void USMSkillCooldownWidget::InitializeCooldownWidget(UAbilitySystemComponent* InASC,
                                                       USMInventoryComponent* InInventoryComponent)
{
    UnbindASCDelegates();
    UnregisterMessageListener();

    BoundASC       = InASC;
    BoundInventory = InInventoryComponent;

    LastKnownActiveSkillTag = FGameplayTag();
    WatchedCooldownTag      = FGameplayTag();
    CachedFinalCooldown     = 0.f;
    CooldownRemaining       = 0.f;
    bIsOnCooldown           = false;

    if (BoundASC && BoundInventory)
    {
        BindASCDelegates();
        RegisterMessageListener();
        RefreshWatchedTag();
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

    // 활성 스킬 태그가 바뀐 경우에만 갱신
    if (BoundInventory)
    {
        const FGameplayTag CurrentActiveTag = BoundInventory->GetActiveSkillTag();
        if (CurrentActiveTag != LastKnownActiveSkillTag)
        {
            LastKnownActiveSkillTag = CurrentActiveTag;
            RefreshWatchedTag();
        }
    }

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

void USMSkillCooldownWidget::HandleQuickSlotUpdated(FGameplayTag,
                                                     const FSMQuickSlotUpdatedMessage& Message)
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
        {
            if (Message.GetOwningPlayerState() != PS) return;
        }
    }
    LastKnownActiveSkillTag = FGameplayTag();
    CachedFinalCooldown     = 0.f; // 슬롯 바뀌면 Duration 캐시 초기화
    RefreshWatchedTag();
}

// ─── 감시 태그 갱신 ──────────────────────────────────────────

void USMSkillCooldownWidget::RefreshWatchedTag()
{
    if (!BoundInventory)
    {
        WatchedCooldownTag  = FGameplayTag();
        CachedFinalCooldown = 0.f;
        bIsOnCooldown       = false;
        UpdateVisuals();
        return;
    }

    const FGameplayTag ActiveSkillTag = BoundInventory->GetActiveSkillTag();
    if (!ActiveSkillTag.IsValid())
    {
        WatchedCooldownTag  = FGameplayTag();
        CachedFinalCooldown = 0.f;
        bIsOnCooldown       = false;
        UpdateVisuals();
        return;
    }

    // "Ability.Skill.Xxx" → "Cooldown.Skill.Xxx" 변환
    const FString ActiveTagStr = ActiveSkillTag.ToString();
    FString CooldownTagStr;

    if (ActiveTagStr.StartsWith(TEXT("Ability.")))
    {
        CooldownTagStr = ActiveTagStr.Replace(TEXT("Ability."), TEXT("Cooldown."));
    }
    else
    {
        int32 DotIdx = INDEX_NONE;
        ActiveTagStr.FindChar(TEXT('.'), DotIdx);
        CooldownTagStr = (DotIdx != INDEX_NONE)
            ? TEXT("Cooldown") + ActiveTagStr.RightChop(DotIdx)
            : TEXT("Cooldown.") + ActiveTagStr;
    }

    const FGameplayTag NewCooldownTag =
        FGameplayTag::RequestGameplayTag(FName(*CooldownTagStr), false);

    // 태그가 바뀌면 Duration 캐시도 초기화 (이전 스킬 값이 남지 않도록)
    if (NewCooldownTag != WatchedCooldownTag)
    {
        CachedFinalCooldown = 0.f;
    }
    WatchedCooldownTag = NewCooldownTag;

    // 인벤토리 캐시에서 FinalCooldown 읽기 시도
    // 데디케이트 서버 환경 클라이언트에서는 대부분 0으로 반환됨
    // → 그래도 시도해서 값이 있으면 사용, 없으면 OnEffectAdded에서 GE Duration으로 채움
    FSMCompiledSkillSummary Summary;
    if (BoundInventory->GetActiveSkillSummary(Summary) && Summary.GetFinalCooldown() > 0.f)
    {
        CachedFinalCooldown = Summary.GetFinalCooldown();
    }
    // CachedFinalCooldown = 0 이어도 여기서 UpdateVisuals 호출하지 않음
    // RefreshCooldownState → GE Duration 폴백에서 채워진 후 UpdateVisuals 호출

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

void USMSkillCooldownWidget::OnEffectAdded(UAbilitySystemComponent*,
                                            const FGameplayEffectSpec& Spec,
                                            FActiveGameplayEffectHandle)
{
    if (!WatchedCooldownTag.IsValid()) return;

    FGameplayTagContainer AllGrantedTags;
    Spec.GetAllGrantedTags(AllGrantedTags);
    if (!AllGrantedTags.HasTag(WatchedCooldownTag)) return;

    // GE가 실제로 추가된 시점에 Duration 확보
    // 인벤 캐시(서버 계산값)가 없을 때의 주 폴백 경로
    if (CachedFinalCooldown <= 0.f)
    {
        const float GEDuration = Spec.GetDuration();
        if (GEDuration > 0.f)
        {
            CachedFinalCooldown = GEDuration;
        }
    }

    // GE 추가 직후 남은 시간 읽기
    RefreshCooldownState();
}

void USMSkillCooldownWidget::OnEffectRemoved(const FActiveGameplayEffect& Effect)
{
    if (!WatchedCooldownTag.IsValid()) return;

    FGameplayTagContainer AllGrantedTags;
    Effect.Spec.GetAllGrantedTags(AllGrantedTags);
    if (!AllGrantedTags.HasTag(WatchedCooldownTag)) return;

    bIsOnCooldown     = false;
    CooldownRemaining = 0.f;
    CachedFinalCooldown = 0.f; // 다음 발동을 위해 초기화
    UpdateVisuals();
}

// ─── 쿨다운 상태 읽기 ────────────────────────────────────────

void USMSkillCooldownWidget::RefreshCooldownState()
{
    if (!BoundASC || !WatchedCooldownTag.IsValid())
    {
        bIsOnCooldown     = false;
        CooldownRemaining = 0.f;
        UpdateVisuals();
        return;
    }

    const FGameplayEffectQuery Query =
        FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(WatchedCooldownTag));

    const TArray<float> Remaining = BoundASC->GetActiveEffectsTimeRemaining(Query);

    if (Remaining.IsEmpty() || Remaining[0] <= 0.f)
    {
        bIsOnCooldown     = false;
        CooldownRemaining = 0.f;
        // CachedFinalCooldown은 유지 (다음 RefreshCooldownState 호출 전까지)
        UpdateVisuals();
        return;
    }

    // GE가 살아있음 → 쿨다운 진행 중
    bIsOnCooldown     = true;
    CooldownRemaining = Remaining[0];

    // CachedFinalCooldown이 아직 0인 경우 (OnEffectAdded보다 이 함수가 먼저 호출된 경우)
    // GE Duration을 직접 읽어서 채움
    if (CachedFinalCooldown <= 0.f)
    {
        const TArray<float> Durations = BoundASC->GetActiveEffectsDuration(Query);
        if (!Durations.IsEmpty() && Durations[0] > 0.f)
        {
            CachedFinalCooldown = Durations[0];
        }
        else
        {
            // Duration도 읽히지 않으면 Remaining을 임시 기준으로
            CachedFinalCooldown = CooldownRemaining;
        }
    }

    CooldownRemaining = FMath::Min(CooldownRemaining, CachedFinalCooldown);
    UpdateVisuals();
}

// ─── UI 반영 ─────────────────────────────────────────────────

void USMSkillCooldownWidget::UpdateVisuals()
{
    const float Percent = (bIsOnCooldown && CachedFinalCooldown > 0.f)
        ? FMath::Clamp(CooldownRemaining / CachedFinalCooldown, 0.f, 1.f)
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

    SetVisibility(bIsOnCooldown
        ? ESlateVisibility::HitTestInvisible
        : ESlateVisibility::Collapsed);
}