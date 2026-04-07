#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SMMonsterData.generated.h"

class ASMMonsterBase;

UENUM()
enum class EMonsterType : uint8
{
    None    UMETA(DisplayName = "없음"),
    Snake   UMETA(DisplayName = "Snake"),
    Squid   UMETA(DisplayName = "Squid"),
    Bird    UMETA(DisplayName = "Bird")
};

USTRUCT(BlueprintType)
struct FSMMonsterData : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Info")
    EMonsterType MonsterType = EMonsterType::None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Info")
    FText MonsterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stat")
    float MaxHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stat")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stat")
    float AttackRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stat")
    float AttackDamage = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Stat")
    float AttackSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Reward")
    int32 DropGold = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster|Asset")
    TSoftClassPtr<ASMMonsterBase> MonsterClass;
};
