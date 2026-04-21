#include "SMHUDManager.h"
#include "UI/SMPlayerStatusWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "SMVolumeWidget.h"
#include "TimerManager.h"
#include "UI/SMGameResultWidget.h"
#include "UI/SMPlayerDeathWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "UI/SMNotificationWidget.h"
#include "UI/SMSkillCooldownWidget.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameplayTags/Message/SMMessageTag.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Inventory/Core/SMInventoryMessageTypes.h"

void USMHUDManager::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Button_Quit)
	{
		Button_Quit->OnClicked.AddDynamic(this, &USMHUDManager::OnQuitButtonClicked);
	}
	
	// 퀵슬롯 변경 구독
	UGameplayMessageSubsystem& MsgSys = UGameplayMessageSubsystem::Get(this);
	QuickSlotListenerHandle = MsgSys.RegisterListener<FSMQuickSlotUpdatedMessage>(
		SMMessageTag::Inventory_QuickSlotUpdated,
		this,
		&USMHUDManager::OnQuickSlotUpdated);

	TryInitASC();
}

void USMHUDManager::NativeDestruct()
{
	if (QuickSlotListenerHandle.IsValid())
	{
		QuickSlotListenerHandle.Unregister();
	}

	if (CachedASC)
	{
		CachedASC->OnActiveGameplayEffectAddedDelegateToSelf.Remove(GEAddedHandle);
		CachedASC->OnAnyGameplayEffectRemovedDelegate().Remove(GERemovedHandle);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
	} 

	Super::NativeDestruct();
}

void USMHUDManager::TryInitASC()
{
	// HUD가 플레이어 폰을 찾아 ASC 연동
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>()) // 폰이 죽어도 살아있어서 더 안정!
		{
			if (APawn* OwningPawn = PC->GetPawn())
			{
				// 캐릭터가 IAbilitySystemInterface를 상속받았는지 확인 후 ASC 가져옴
				if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwningPawn))
				{
					UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
					if (ASC && ASC->GetAvatarActor() == OwningPawn)
					{
						InitializeHUD(ASC);
						// 연동 성공 -> 타이머 해제
						GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
						return;
					}
				}
			}
		}
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(ASC_InitTimerHandle, this, &USMHUDManager::TryInitASC, 0.1f, false);
	}
}

void USMHUDManager::InitializeHUD(UAbilitySystemComponent* InPlayerASC)
{
	if (!InPlayerASC) return;

	if (CachedASC)
	{
		CachedASC->OnActiveGameplayEffectAddedDelegateToSelf.Remove(GEAddedHandle);
		CachedASC->OnAnyGameplayEffectRemovedDelegate().Remove(GERemovedHandle);
		GEAddedHandle.Reset();
		GERemovedHandle.Reset();
	}

	CachedASC = InPlayerASC;

	if (WBP_PlayerStatus)
	{
		WBP_PlayerStatus->InitializeStatus(InPlayerASC);
	}

	// GE 추가될 때마다 쿨타임 위젯 갱신
	GEAddedHandle = InPlayerASC->OnActiveGameplayEffectAddedDelegateToSelf.AddLambda(
		[this](UAbilitySystemComponent*, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle)
		{
			FGameplayTagContainer GrantedTags;
			Spec.GetAllGrantedTags(GrantedTags);

			for (const FGameplayTag& Tag : GrantedTags)
			{
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Skill"))))
				{
					RefreshCooldownWidget(CachedASC);
					return;
				}
			}
		}
	);

	// GE 제거될 때도 갱신
	GERemovedHandle = InPlayerASC->OnAnyGameplayEffectRemovedDelegate().AddLambda(
		[this](const FActiveGameplayEffect& RemovedEffect)
		{
			FGameplayTagContainer GrantedTags;
			RemovedEffect.Spec.GetAllGrantedTags(GrantedTags);

			for (const FGameplayTag& Tag : GrantedTags)
			{
				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Cooldown.Skill"))))
				{
					RefreshCooldownWidget(CachedASC);
					return;
				}
			}
		}
	);

	RefreshCooldownWidget(InPlayerASC);
}

void USMHUDManager::RefreshHUD(UAbilitySystemComponent* InPlayerASC)
{
	InitializeHUD(InPlayerASC);
}

void USMHUDManager::ShowGameResult(bool bIsVictory, float InReturnDelay)
{
	// 다른 HUD 위젯 숨기기
	if (WBP_PlayerStatus)
	{
		WBP_PlayerStatus->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (WBP_PlayerDeath)
	{
		WBP_PlayerDeath->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (WBP_Notification)
	{
		WBP_Notification->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (WBP_SkillCooldown)
	{
		WBP_SkillCooldown->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (Button_Quit)
	{
		Button_Quit->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (WBP_Volume)
	{
		WBP_Volume->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (WBP_GameResult)
	{
		WBP_GameResult->SetVisibility(ESlateVisibility::Visible);
		WBP_GameResult->ShowResult(bIsVictory, InReturnDelay);
	}
}

void USMHUDManager::ShowPlayerDeath(float RespawnTime)
{
	if (WBP_PlayerDeath)
	{
		WBP_PlayerDeath->ShowDeathWidget(RespawnTime);
	}
}

void USMHUDManager::HidePlayerDeath()
{
	if (WBP_PlayerDeath)
	{
		WBP_PlayerDeath->HideDeathWidget();
	}
}

void USMHUDManager::OnQuitButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}

void USMHUDManager::RefreshCooldownWidget(UAbilitySystemComponent* InPlayerASC)
{
	if (!WBP_SkillCooldown || !InPlayerASC) return;

    // 모든 쿨타임 태그 목록
    static const TArray<FGameplayTag> AllCooldownTags =
    {
        SMSkillTag::Cooldown_Skill_Projectile,
        SMSkillTag::Cooldown_Skill_SpawnField,
        SMSkillTag::Cooldown_Skill_LineTrace,
        SMSkillTag::Cooldown_Skill_ApplyInstantDamage,
        SMSkillTag::Cooldown_Skill_Explosion,
        SMSkillTag::Cooldown_Skill_SummonTurret,
    };
	
    FGameplayTag ActiveCooldownTag;
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
        {
            if (USMInventoryComponent* Inv = PS->FindComponentByClass<USMInventoryComponent>())
            {
                FGameplayTag ActiveSkillTag = Inv->GetActiveSkillTag();
                if (ActiveSkillTag.IsValid())
                {
                    FString TagString = ActiveSkillTag.ToString();
                    TagString = TagString.Replace(TEXT("Ability.Skill."), TEXT("Cooldown.Skill."));
                    ActiveCooldownTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
                }
            }
        }
    }
	
    if (ActiveCooldownTag.IsValid())
    {
        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(ActiveCooldownTag));
        if (InPlayerASC->GetActiveEffects(Query).Num() > 0)
        {
            WBP_SkillCooldown->InitializeWithASC(InPlayerASC, ActiveCooldownTag);
            return;
        }
    }
	
    for (const FGameplayTag& Tag : AllCooldownTags)
    {
        if (Tag == ActiveCooldownTag) continue;

        FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(
            FGameplayTagContainer(Tag));
        if (InPlayerASC->GetActiveEffects(Query).Num() > 0)
        {
            WBP_SkillCooldown->InitializeWithASC(InPlayerASC, Tag);
            return;
        }
    }

    // 살아있는 쿨타임이 아무것도 없으면 위젯 숨김
    WBP_SkillCooldown->InitializeWithASC(InPlayerASC, FGameplayTag());
}

void USMHUDManager::OnQuickSlotUpdated(FGameplayTag InChannel, const FSMQuickSlotUpdatedMessage& InMessage)
{
	// 내 플레이어 것인지 확인
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
		{
			if (InMessage.GetOwningPlayerState() != PS) return;
		}
	}

	if (CachedASC)
	{
		RefreshCooldownWidget(CachedASC);
	}
}