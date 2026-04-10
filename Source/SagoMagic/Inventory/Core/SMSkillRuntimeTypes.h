#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SMSkillRuntimeTypes.generated.h"


/**
 * 스킬 인벤토리 계산 결과 및 스킬 요약 캐시 데이터 Struct 정의 파일
 *
 * 포함 내용:
 * - 스킬 레벨
 * - 최종 데미지
 * - 최종 사거리/범위
 * - 최종 쿨타임
 * - 특수 동작 태그
 * - 장착된 젬/보조 스킬 ID 목록(디버그/추적용)
 *
 * 역할:
 * - 스킬 내부 장착 결과를 실행 계층에서 바로 사용할 수 있는 형태로 캐싱
 * - UI 및 추후 퀵슬롯/GA 실행 계층에 동일한 요약 데이터 제공
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
		  , FinalDamage(0.0f)
		  , FinalRangeOrArea(0.0f)
		  , FinalCooldown(0.0f)
	{
	}

	/** 현재 레벨 Getter */
	int32 GetCurrentLevel() const
	{
		return CurrentLevel;
	}

	/** 최종 데미지 Getter */
	float GetFinalDamage() const
	{
		return FinalDamage;
	}

	/** 최종 사거리/범위 Getter */
	float GetFinalRangeOrArea() const
	{
		return FinalRangeOrArea;
	}

	/** 최종 쿨타임 Getter */
	float GetFinalCooldown() const
	{
		return FinalCooldown;
	}

	/** 특수 동작 태그 Getter */
	const FGameplayTagContainer& GetBehaviorTags() const
	{
		return BehaviorTags;
	}

	/** 현재 레벨 Setter */
	void SetCurrentLevel(const int32 InCurrentLevel)
	{
		CurrentLevel = InCurrentLevel;
	}

	/** 최종 데미지 Setter */
	void SetFinalDamage(const float InFinalDamage)
	{
		FinalDamage = InFinalDamage;
	}

	/** 최종 사거리/범위 Setter */
	void SetFinalRangeOrArea(const float InFinalRangeOrArea)
	{
		FinalRangeOrArea = InFinalRangeOrArea;
	}

	/** 최종 쿨타임 Setter */
	void SetFinalCooldown(const float InFinalCooldown)
	{
		FinalCooldown = InFinalCooldown;
	}

	/** 특수 동작 태그 Setter */
	void SetBehaviorTags(const FGameplayTagContainer& InBehaviorTags)
	{
		BehaviorTags = InBehaviorTags;
	}

public:
	/** 요약 정보 초기화 */
	void Reset()
	{
		CurrentLevel = 1;
		FinalDamage = 0.0f;
		FinalRangeOrArea = 0.0f;
		FinalCooldown = 0.0f;
		BehaviorTags.Reset();
		EmbeddedGemIds.Reset();
		EmbeddedSkillIds.Reset();
	}

public:
	/** 현재 계산된 스킬 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	int32 CurrentLevel;

	/** 장착 결과 반영 후 최종 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	float FinalDamage;

	/** 장착 결과 반영 후 최종 사거리/범위 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	float FinalRangeOrArea;

	/** 장착 결과 반영 후 최종 쿨타임 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	float FinalCooldown;

	/** 특수 동작 분기용 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill")
	FGameplayTagContainer BehaviorTags;

	/** 디버그/추적용 장착 젬 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill|Debug")
	TArray<FGuid> EmbeddedGemIds;

	/** 디버그/추적용 장착 동일 스킬 인스턴스 ID 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Skill|Debug")
	TArray<FGuid> EmbeddedSkillIds;
};
