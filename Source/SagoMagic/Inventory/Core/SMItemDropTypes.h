#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "SMItemDropTypes.generated.h"

class USMItemDefinition;


/**
 * 내부 장착 아이템 드랍 스냅샷 Struct 정의
 *
 * 포함 내용:
 * - 내부 아이템 인스턴스 ID
 * - 부모 스킬 인스턴스 ID
 * - 아이템 종류
 * - 정의 에셋 참조
 * - 내부 컨테이너 내 Grid 좌표
 * - 회전 상태
 * - 잠금 상태
 *
 * 역할:
 * - 스킬 내부 인벤토리 복원을 위한 평면 스냅샷 데이터 제공
 */

/** 내부 장착 아이템 드랍 스냅샷 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMNestedItemDropSnapshot
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMNestedItemDropSnapshot()
		: ItemType(ESMItemType::None)
		  , GridX(0)
		  , GridY(0)
		  , Rotation(ESMGridRotation::Rot0)
		  , bLocked(false)
	{
	}

	/** 인스턴스 ID Getter */
	const FGuid& GetInstanceId() const
	{
		return InstanceId;
	}

	/** 부모 스킬 인스턴스 ID Getter */
	const FGuid& GetParentSkillInstanceId() const
	{
		return ParentSkillInstanceId;
	}

	/** 아이템 정의 Getter */
	const TSoftObjectPtr<USMItemDefinition>& GetDefinition() const
	{
		return Definition;
	}

	/** Grid X Getter */
	int32 GetGridX() const
	{
		return GridX;
	}

	/** Grid Y Getter */
	int32 GetGridY() const
	{
		return GridY;
	}

	/** 회전값 Getter */
	ESMGridRotation GetRotation() const
	{
		return Rotation;
	}

	/** 잠금 상태 Getter */
	bool IsLocked() const
	{
		return bLocked;
	}

	/** 인스턴스 ID Setter */
	void SetInstanceId(const FGuid& InInstanceId)
	{
		InstanceId = InInstanceId;
	}

	/** 부모 스킬 인스턴스 ID Setter */
	void SetParentSkillInstanceId(const FGuid& InParentSkillInstanceId)
	{
		ParentSkillInstanceId = InParentSkillInstanceId;
	}

	/** 아이템 정의 Setter */
	void SetDefinition(const TSoftObjectPtr<USMItemDefinition>& InDefinition)
	{
		Definition = InDefinition;
	}

	/** Grid X Setter */
	void SetGridX(const int32 InGridX)
	{
		GridX = InGridX;
	}

	/** Grid Y Setter */
	void SetGridY(const int32 InGridY)
	{
		GridY = InGridY;
	}

	/** 회전값 Setter */
	void SetRotation(const ESMGridRotation InRotation)
	{
		Rotation = InRotation;
	}

	/** 잠금 상태 Setter */
	void SetLocked(const bool bInLocked)
	{
		bLocked = bInLocked;
	}

public:
	/** 스냅샷 유효성 검사 */
	bool IsValidSnapshot() const
	{
		return InstanceId.IsValid() && ParentSkillInstanceId.IsValid() && !Definition.IsNull();
	}

public:
	/** 내부 아이템 인스턴스 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	FGuid InstanceId;

	/** 이 아이템을 직접 품고 있던 부모 스킬 인스턴스 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	FGuid ParentSkillInstanceId;

	/** 드랍 대상 아이템 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMItemType ItemType;

	/** 드랍 대상 아이템 정의 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	TSoftObjectPtr<USMItemDefinition> Definition;

	/** 내부 인벤토리 내 Grid X 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	int32 GridX;

	/** 내부 인벤토리 내 Grid Y 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	int32 GridY;

	/** 드랍 당시 회전 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMGridRotation Rotation;

	/** 드랍 당시 잠금 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	bool bLocked;
};


/**
 * 월드 드랍 아이템 복원용 Payload Struct 정의
 *
 * 포함 내용:
 * - 루트 아이템 인스턴스 ID
 * - 아이템 종류
 * - 정의 에셋 참조
 * - 회전 상태
 * - 잠금 상태
 * - 내부 장착 아이템 스냅샷 배열
 *
 * 역할:
 * - 인벤토리에서 월드 드랍으로 전환될 때 필요한 완전 복원 데이터 제공
 */

/** 월드 드랍 복원용 Payload 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMItemDropPayload
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMItemDropPayload()
		: ItemType(ESMItemType::None)
		  , Rotation(ESMGridRotation::Rot0)
		  , bLocked(false)
	{
	}

	/** 인스턴스 ID Getter */
	const FGuid& GetInstanceId() const
	{
		return InstanceId;
	}

	/** 아이템 정의 Getter */
	const TSoftObjectPtr<USMItemDefinition>& GetDefinition() const
	{
		return Definition;
	}

	/** 회전값 Getter */
	ESMGridRotation GetRotation() const
	{
		return Rotation;
	}

	/** 잠금 상태 Getter */
	bool IsLocked() const
	{
		return bLocked;
	}

	/** 내부 아이템 스냅샷 배열 Getter */
	const TArray<FSMNestedItemDropSnapshot>& GetNestedItemSnapshots() const
	{
		return NestedItemSnapshots;
	}

	/** 인스턴스 ID Setter */
	void SetInstanceId(const FGuid& InInstanceId)
	{
		InstanceId = InInstanceId;
	}

	/** 아이템 정의 Setter */
	void SetDefinition(const TSoftObjectPtr<USMItemDefinition>& InDefinition)
	{
		Definition = InDefinition;
	}

	/** 회전값 Setter */
	void SetRotation(const ESMGridRotation InRotation)
	{
		Rotation = InRotation;
	}

	/** 잠금 상태 Setter */
	void SetLocked(const bool bInLocked)
	{
		bLocked = bInLocked;
	}

	/** 내부 아이템 스냅샷 배열 Setter */
	void SetNestedItemSnapshots(const TArray<FSMNestedItemDropSnapshot>& InNestedItemSnapshots)
	{
		NestedItemSnapshots = InNestedItemSnapshots;
	}

public:
	/** Payload 유효성 검사 */
	bool IsValidPayload() const
	{
		if (InstanceId.IsValid() == false || Definition.IsNull())
		{
			return false;
		}

		for (const FSMNestedItemDropSnapshot& Snapshot : NestedItemSnapshots)
		{
			if (Snapshot.IsValidSnapshot() == false)
			{
				return false;
			}
		}

		return true;
	}

public:
	/** 드랍 대상 루트 아이템 인스턴스 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	FGuid InstanceId;

	/** 드랍 대상 루트 아이템 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMItemType ItemType;

	/** 드랍 대상 루트 아이템 정의 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	TSoftObjectPtr<USMItemDefinition> Definition;

	/** 드랍 당시 루트 아이템 회전 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMGridRotation Rotation;

	/** 드랍 당시 루트 아이템 잠금 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	bool bLocked;

	/** 내부 장착 아이템 복원 스냅샷 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	TArray<FSMNestedItemDropSnapshot> NestedItemSnapshots;
};
