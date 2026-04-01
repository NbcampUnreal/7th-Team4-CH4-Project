#pragma once

#include "CoreMinimal.h"
#include "SMGameEnums.generated.h"


UENUM(BlueprintType)
enum class EGemBuffType : uint8
{
    None				UMETA(DisplayName = "없음"),
    DamageMultiplier	UMETA(DisplayName = "데미지 배율 증가"),
    CooldownReduction	UMETA(DisplayName = "쿨타임 감소"),
    ProjectileRangee    UMETA(DisplayName = "투사체 범위 증가"),
    // AttackSpeedBoost	UMETA(DisplayName = "공격 속도 증가")
};
