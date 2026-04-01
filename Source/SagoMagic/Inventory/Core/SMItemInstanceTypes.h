#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Inventory/Core/SMSkillRuntimeTypes.h"
#include "SMItemInstanceTypes.generated.h"

class USMItemDefinition;

/**
 * 아이템 인스턴스 런타임 상태 Struct 정의 파일
 *
 * 포함 내용:
 * - 공통 아이템 인스턴스 데이터
 * - 젬용 아이템 인스턴스 데이터
 * - 스킬용 아이템 인스턴스 데이터
 *
 * 역할:
 * - 실제 플레이 중 존재하는 아이템 개체(인스턴스)의 위치, 회전, 부모 컨테이너, 내부 장착 상태 등을 저장
 */

/** 공통 아이템 인스턴스 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMItemInstanceData
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMItemInstanceData()
		: ItemType(ESMItemType::None)
		, GridX(0)
		, GridY(0)
		, Rotation(0)
		, bLocked(false)
	{
	}

	/** 인스턴스 ID Getter */
	const FGuid& GetInstanceId() const
	{
		return InstanceId;
	}

	/** 아이템 타입 Getter */
	ESMItemType GetItemType() const
	{
		return ItemType;
	}

	/** 아이템 정의 Getter */
	const TSoftObjectPtr<USMItemDefinition>& GetDefinition() const
	{
		return Definition;
	}

	/** 부모 컨테이너 ID Getter */
	const FGuid& GetParentContainerId() const
	{
		return ParentContainerId;
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
	int32 GetRotation() const
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

	/** 아이템 타입 Setter */
	void SetItemType(const ESMItemType InItemType)
	{
		ItemType = InItemType;
	}

	/** 아이템 정의 Setter */
	void SetDefinition(const TSoftObjectPtr<USMItemDefinition>& InDefinition)
	{
		Definition = InDefinition;
	}

	/** 부모 컨테이너 ID Setter */
	void SetParentContainerId(const FGuid& InParentContainerId)
	{
		ParentContainerId = InParentContainerId;
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
	void SetRotation(const int32 InRotation)
	{
		Rotation = InRotation;
	}

	/** 잠금 상태 Setter */
	void SetLocked(const bool bInLocked)
	{
		bLocked = bInLocked;
	}

public:
	/** 인스턴스 데이터 유효성 검사 */
	bool IsValidInstance() const
	{
		return InstanceId.IsValid() && !Definition.IsNull();
	}

public:
	/** 아이템 인스턴스 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	FGuid InstanceId;

	/** 아이템 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	ESMItemType ItemType;

	/** 참조 아이템 정의 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	TSoftObjectPtr<USMItemDefinition> Definition;

	/** 부모 컨테이너 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	FGuid ParentContainerId;

	/** 현재 Grid X 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	int32 GridX;

	/** 현재 Grid Y 좌표 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	int32 GridY;

	/** 현재 회전 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	int32 Rotation;

	/** 잠금 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Item")
	bool bLocked;
};

/** 젬 아이템 인스턴스 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMGemItemInstanceData
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMGemItemInstanceData() = default;

	/** 공통 아이템 인스턴스 Getter */
	const FSMItemInstanceData& GetBaseItem() const
	{
		return BaseItem;
	}

	/** 공통 아이템 인스턴스 Setter */
	void SetBaseItem(const FSMItemInstanceData& InBaseItem)
	{
		BaseItem = InBaseItem;
	}

public:
	/** 공통 아이템 인스턴스 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Gem")
	FSMItemInstanceData BaseItem;
};

/** 스킬 아이템 인스턴스 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMSkillItemInstanceData
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMSkillItemInstanceData() = default;

	/** 내부 컨테이너 ID Getter */
	const FGuid& GetInternalContainerId() const
	{
		return InternalContainerId;
	}

	/** 요약 캐시 Getter */
	const FSMCompiledSkillSummary& GetCachedSummary() const
	{
		return CachedSummary;
	}

	/** 내부 컨테이너 ID Setter */
	void SetInternalContainerId(const FGuid& InInternalContainerId)
	{
		InternalContainerId = InInternalContainerId;
	}

	/** 요약 캐시 Setter */
	void SetCachedSummary(const FSMCompiledSkillSummary& InCachedSummary)
	{
		CachedSummary = InCachedSummary;
	}

public:
	/** 스킬 인스턴스 유효성 검사 */
	bool IsValidSkillInstance() const
	{
		return BaseItem.IsValidInstance() && InternalContainerId.IsValid();
	}

public:
	/** 공통 아이템 인스턴스 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	FSMItemInstanceData BaseItem;

	/** 스킬 내부 인벤토리 컨테이너 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	FGuid InternalContainerId;

	/** 내부 장착 아이템 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	TArray<FGuid> EmbeddedItemIds;

	/** 계산된 스킬 요약 캐시 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	FSMCompiledSkillSummary CachedSummary;
};
