#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "SMItemDropTableData.generated.h"


/**
 * 공용 아이템 드랍 테이블 Row 정의 파일
 *
 * 포함 내용:
 * - 드랍 대상 아이템 정의 에셋
 * - 가중치 기반 드랍 확률 값
 * - 드랍 데이터 유효성 검사 함수
 *
 * 역할:
 * - 몬스터 사망 시 공용 드랍 테이블에서 사용할 최소 단위 데이터 제공
 * - 모든 몬스터가 동일한 드랍 테이블을 참조하는 초기 구조 지원
 */

/** 공용 아이템 드랍 테이블 Row */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMItemDropTableData : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMItemDropTableData()
		: DropWeight(1)
	{
	}

	/** 드랍 대상 아이템 정의 Getter */
	const TSoftObjectPtr<USMItemDefinition>& GetItemDefinition() const
	{
		return ItemDefinition;
	}

	/** 드랍 가중치 Getter */
	int32 GetDropWeight() const
	{
		return DropWeight;
	}

	/** 드랍 대상 아이템 정의 Setter */
	void SetItemDefinition(const TSoftObjectPtr<USMItemDefinition>& InItemDefinition)
	{
		ItemDefinition = InItemDefinition;
	}

	/** 드랍 가중치 Setter */
	void SetDropWeight(const int32 InDropWeight)
	{
		DropWeight = InDropWeight;
	}

public:
	/** Row 유효성 검사 */
	bool IsValidData() const
	{
		return ItemDefinition.IsNull() == false && DropWeight > 0;
	}

public:
	/** 드랍 대상 아이템 정의 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Drop Table")
	TSoftObjectPtr<USMItemDefinition> ItemDefinition;

	/** 가중치 기반 드랍 확률 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Drop Table", meta=(ClampMin="0", UIMin="0"))
	int32 DropWeight;
};
