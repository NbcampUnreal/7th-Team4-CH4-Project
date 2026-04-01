#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMSkillProgressionFragment.generated.h"


/**
 * 스킬 성장 Fragment 정의 파일
 *
 * 포함 내용:
 * - 기본 레벨
 * - 동일 이름 스킬 장착 시 레벨 증가 여부
 * - 최대 레벨
 *
 * 역할:
 * - 스킬 아이템 레벨 계산 규칙 제공
 */

/** 스킬 성장 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMSkillProgressionFragment : public USMItemFragment
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMSkillProgressionFragment()
        : BaseLevel(1)
        , bLevelFromEmbeddedSameSkill(true)
        , MaxLevel(99)
    {
    }

    /** 기본 레벨 Getter */
    int32 GetBaseLevel() const
    {
        return BaseLevel;
    }

    /** 동일 이름 스킬 장착 시 레벨 증가 여부 Getter */
    bool IsLevelFromEmbeddedSameSkill() const
    {
        return bLevelFromEmbeddedSameSkill;
    }

    /** 최대 레벨 Getter */
    int32 GetMaxLevel() const
    {
        return MaxLevel;
    }

    /** 기본 레벨 Setter */
    void SetBaseLevel(const int32 InBaseLevel)
    {
        BaseLevel = InBaseLevel;
    }

    /** 동일 이름 스킬 장착 시 레벨 증가 여부 Setter */
    void SetLevelFromEmbeddedSameSkill(const bool bInLevelFromEmbeddedSameSkill)
    {
        bLevelFromEmbeddedSameSkill = bInLevelFromEmbeddedSameSkill;
    }

    /** 최대 레벨 Setter */
    void SetMaxLevel(const int32 InMaxLevel)
    {
        MaxLevel = InMaxLevel;
    }

public:
    /** 기본 레벨 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Progression Fragment")
    int32 BaseLevel;

    /** 동일 이름 스킬 장착 시 레벨 증가 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Progression Fragment")
    bool bLevelFromEmbeddedSameSkill;

    /** 최대 레벨 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Skill Progression Fragment")
    int32 MaxLevel;
};
