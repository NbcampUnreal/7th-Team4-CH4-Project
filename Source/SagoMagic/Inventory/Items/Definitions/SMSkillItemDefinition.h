#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "SMSkillItemDefinition.generated.h"

struct FSMSkillData;


/**
 * 스킬 아이템 정의 DataAsset 클래스 정의 파일
 *
 * 포함 내용:
 * - 스킬 전용 정의 클래스
 * - 스킬 실행 데이터 테이블 행 참조
 *
 * 역할:
 * - 스킬 아이템 에셋 생성 및 타입 분리용 정의 제공
 * - 스킬 요약 계산 시 사용할 기본/레벨 데이터 연결점 제공
 */

/** 스킬 아이템 정의 DataAsset 클래스 */
UCLASS(BlueprintType)
class SAGOMAGIC_API USMSkillItemDefinition : public USMItemDefinition
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMSkillItemDefinition() = default;

	/** 스킬 실행 데이터 행 참조 Getter */
	const FDataTableRowHandle& GetSkillStatRow() const
	{
		return SkillStatRow;
	}

	/** 스킬 실행 데이터 행 참조 Setter */
	void SetSkillStatRow(const FDataTableRowHandle& InSkillStatRow)
	{
		SkillStatRow = InSkillStatRow;
	}

public:
	/** 요약 계산에 사용할 스킬 기본/레벨 데이터 테이블 행 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Definition")
	FDataTableRowHandle SkillStatRow;
};
