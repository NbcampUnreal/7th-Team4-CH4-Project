#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "SMItemDropTypes.generated.h"

class USMItemDefinition;


/**
 * 월드 드랍 아이템 복원용 Payload Struct 정의 파일
 *
 * 포함 내용:
 * - 드랍 대상 아이템 인스턴스 ID
 * - 아이템 종류
 * - 정의 에셋 참조
 * - 회전 상태
 * - 내부 포함 아이템 ID 목록
 *
 * 역할:
 * - 인벤토리에서 월드 드랍으로 전환될 때 필요한 최소 복원 데이터 제공
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

public:
	/** Payload 유효성 검사 */
	bool IsValidPayload() const
	{
		return InstanceId.IsValid() && !Definition.IsNull();
	}

public:
	/** 드랍 대상 아이템 인스턴스 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	FGuid InstanceId;

	/** 드랍 대상 아이템 종류 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMItemType ItemType;

	/** 드랍 대상 아이템 정의 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	TSoftObjectPtr<USMItemDefinition> Definition;

	/** 드랍 당시 회전 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	ESMGridRotation Rotation;

	/** 내부 포함 아이템 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Drop")
	TArray<FGuid> NestedItemIds;
};
