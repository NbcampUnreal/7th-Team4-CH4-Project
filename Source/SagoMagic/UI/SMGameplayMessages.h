#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SMGameplayMessages.generated.h"


UENUM(BlueprintType)
enum class EWaveUIState : uint8
{
    Preparing,
    Started,
    Inprogress,
    Cleared,
};

UENUM(BlueprintType)
enum class EBaseCampEvent : uint8
{
    Attacked,
    Repaired,
    Destroyed,
};

/**
 * UI.Event.Wave
 * Listener : HUDManager, WaveStatusWidget
 */
USTRUCT(BlueprintType)
struct FWaveMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EWaveUIState State = EWaveUIState::Preparing;
    UPROPERTY(BlueprintReadOnly)
    int32 WaveIndex = 0;
    UPROPERTY(BlueprintReadOnly)
    float TimeRemaining = 0.0f;
};

/**
 * UI.Event.Player
 * Listener :
 * PlayerHP, Gold는 GAS 델리게이트로 처리
 */
USTRUCT(BlueprintType)
struct FPlayerStatusMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 PlayerIndex = 0;
    UPROPERTY(BlueprintReadOnly)
    float RespawnTime = 5.0f;
};

/**
 * UI.Event.BaseCamp
 * Listener :
 */
USTRUCT(BlueprintType)
struct FBaseCampMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    EBaseCampEvent EventType = EBaseCampEvent::Attacked;
    UPROPERTY(BlueprintReadOnly)
    float BaseCampHP = 1.0f;
    UPROPERTY(BlueprintReadOnly)
    float CurrentHP = 100.0f;
    UPROPERTY(BlueprintReadOnly)
    float MaxHP = 100.0f;
};

/**
 * UI.Event.QuickSlot
 * Listener :
 */
USTRUCT(BlueprintType)
struct FQuickSlotMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 SlotIndex = 0;
    UPROPERTY(BlueprintReadOnly)
    FGameplayTag AbilityTag;
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<UTexture2D> SkillIcon;
};

/**
 * UI.Event.Result
 * Listener :
 */
USTRUCT(BlueprintType)
struct FResultMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bIsVictory = false;
    UPROPERTY(BlueprintReadOnly)
    int32 TotalKillCount = 0;
    UPROPERTY(BlueprintReadOnly)
    int32 EarnedGold = 0;
};

/**
 * UI.Event.SystemLog
 * Listener :
 */
USTRUCT(BlueprintType)
struct FSystemMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName LogID;
    UPROPERTY(BlueprintReadOnly)
    float LogDuration = 0.0f;
};

/**
 * UI.Event.Boss
 * Listener :
 */
USTRUCT(BlueprintType)
struct FBossMsg
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FName BossName;
    UPROPERTY(BlueprintReadOnly)
    int32 MaxPhase = 2;
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPhase = 1;
    UPROPERTY(BlueprintReadOnly)
    float HPPercent = 1.f;
};
