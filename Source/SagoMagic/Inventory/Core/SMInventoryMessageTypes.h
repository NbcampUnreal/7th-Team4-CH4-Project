#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SMInventoryMessageTypes.generated.h"


/**
 * 인벤토리 Gameplay Message Payload Struct 정의 파일
 *
 * 포함 내용:
 * - 인벤토리 갱신 메시지
 * - 스킬 요약 갱신 메시지
 * - 퀵슬롯 갱신 메시지
 *
 * 역할:
 * - Gameplay Message Subsystem을 통한 UI 갱신용 Payload 데이터 제공
 */

/** 인벤토리 갱신 메시지 Payload */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMInventoryUpdatedMessage
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMInventoryUpdatedMessage() = default;

	/** 소유 PlayerState Getter */
	APlayerState* GetOwningPlayerState() const
	{
		return OwningPlayerState;
	}

	/** 갱신 컨테이너 ID Getter */
	const FGuid& GetContainerId() const
	{
		return ContainerId;
	}

	/** 소유 PlayerState Setter */
	void SetOwningPlayerState(APlayerState* InOwningPlayerState)
	{
		OwningPlayerState = InOwningPlayerState;
	}

	/** 갱신 컨테이너 ID Setter */
	void SetContainerId(const FGuid& InContainerId)
	{
		ContainerId = InContainerId;
	}

public:
	/** 소유 PlayerState */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	TObjectPtr<APlayerState> OwningPlayerState = nullptr;

	/** 갱신 대상 컨테이너 ID */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	FGuid ContainerId;
};

/** 스킬 요약 갱신 메시지 Payload */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMSkillSummaryUpdatedMessage
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMSkillSummaryUpdatedMessage() = default;

	/** 소유 PlayerState Getter */
	APlayerState* GetOwningPlayerState() const
	{
		return OwningPlayerState;
	}

	/** 스킬 인스턴스 ID Getter */
	const FGuid& GetSkillInstanceId() const
	{
		return SkillInstanceId;
	}

	/** 소유 PlayerState Setter */
	void SetOwningPlayerState(APlayerState* InOwningPlayerState)
	{
		OwningPlayerState = InOwningPlayerState;
	}

	/** 스킬 인스턴스 ID Setter */
	void SetSkillInstanceId(const FGuid& InSkillInstanceId)
	{
		SkillInstanceId = InSkillInstanceId;
	}

public:
	/** 소유 PlayerState */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	TObjectPtr<APlayerState> OwningPlayerState = nullptr;

	/** 갱신 대상 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	FGuid SkillInstanceId;
};

/** 퀵슬롯 갱신 메시지 Payload */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMQuickSlotUpdatedMessage
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMQuickSlotUpdatedMessage()
		: SlotIndex(INDEX_NONE)
	{
	}

	/** 소유 PlayerState Getter */
	APlayerState* GetOwningPlayerState() const
	{
		return OwningPlayerState;
	}

	/** 슬롯 인덱스 Getter */
	int32 GetSlotIndex() const
	{
		return SlotIndex;
	}

	/** 소유 PlayerState Setter */
	void SetOwningPlayerState(APlayerState* InOwningPlayerState)
	{
		OwningPlayerState = InOwningPlayerState;
	}

	/** 슬롯 인덱스 Setter */
	void SetSlotIndex(const int32 InSlotIndex)
	{
		SlotIndex = InSlotIndex;
	}

public:
	/** 소유 PlayerState */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	TObjectPtr<APlayerState> OwningPlayerState = nullptr;

	/** 갱신 대상 슬롯 인덱스 */
	UPROPERTY(BlueprintReadWrite, Category="Inventory Message")
	int32 SlotIndex;
};
