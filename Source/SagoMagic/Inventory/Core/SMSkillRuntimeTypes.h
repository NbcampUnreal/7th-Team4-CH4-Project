#pragma once

#include "CoreMinimal.h"
#include "SMSkillRuntimeTypes.generated.h"


/**
 * 스킬 인벤토리 계산 결과 및 스킬 요약 캐시 데이터 Struct 정의 파일
 *
 * 포함 내용:
 * - 스킬 레벨
 * - 총합 효과
 * - 총합 사거리/범위
 * - 총합 쿨타임
 * - 장착된 젬/보조 스킬 ID 목록
 * 추후 추가 가능성 있음
 *
 * 역할:
 * - 스킬 내부 장착 결과를 런타임에서 캐싱하고 UI 및 실행 계층에 전달하는 데이터 제공
 */

/** 스킬 계산 결과 캐시 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMCompiledSkillSummary
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMCompiledSkillSummary()
		: CurrentLevel(1)
		, TotalEffect(0)
		, TotalRangeOrArea(0)
		, TotalCooldown(0)
	{
	}

	/** 현재 레벨 Getter */
	int32 GetCurrentLevel() const
	{
		return CurrentLevel;
	}

	/** 총합 효과 Getter */
	int32 GetTotalEffect() const
	{
		return TotalEffect;
	}

	/** 총합 사거리/범위 Getter */
	int32 GetTotalRangeOrArea() const
	{
		return TotalRangeOrArea;
	}

	/** 총합 쿨타임 Getter */
	int32 GetTotalCooldown() const
	{
		return TotalCooldown;
	}

	/** 현재 레벨 Setter */
	void SetCurrentLevel(const int32 InCurrentLevel)
	{
		CurrentLevel = InCurrentLevel;
	}

	/** 총합 효과 Setter */
	void SetTotalEffect(const int32 InTotalEffect)
	{
		TotalEffect = InTotalEffect;
	}

	/** 총합 사거리/범위 Setter */
	void SetTotalRangeOrArea(const int32 InTotalRangeOrArea)
	{
		TotalRangeOrArea = InTotalRangeOrArea;
	}

	/** 총합 쿨타임 Setter */
	void SetTotalCooldown(const int32 InTotalCooldown)
	{
		TotalCooldown = InTotalCooldown;
	}

public:
	/** 요약 정보 초기화 */
	void Reset()
	{
		CurrentLevel = 1;
		TotalEffect = 0;
		TotalRangeOrArea = 0;
		TotalCooldown = 0;
		EmbeddedGemIds.Reset();
		EmbeddedSkillIds.Reset();
	}

public:
	/** 현재 계산된 스킬 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	int32 CurrentLevel;

	/** 장착 젬 기반 총합 효과 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	int32 TotalEffect;

	/** 장착 젬 기반 총합 사거리/범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	int32 TotalRangeOrArea;

	/** 장착 젬 기반 총합 쿨타임 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	int32 TotalCooldown;

	/** 장착 젬 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	TArray<FGuid> EmbeddedGemIds;

	/** 장착 동일 스킬 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	TArray<FGuid> EmbeddedSkillIds;
};
