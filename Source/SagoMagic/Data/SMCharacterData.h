#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SMCharacterData.generated.h"


USTRUCT(BlueprintType)
struct FSMCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Info")
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stat")
    float MaxHP = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Stat")
    float MoveSpeed = 600.0f;

    /** 추후 데미지 공식 ex. (CharacterBaseAttack + SkillBaseDamage) * GemMultiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Combat")
    float BaseAttack = 10.0f;

    /** 추후 받는 데미지 공식 ex. MonsterAttackDamage - CharacterBaseDefence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Combat")
    float BaseDefense = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|State")
    float RespawnTime = 5.0f;
};
