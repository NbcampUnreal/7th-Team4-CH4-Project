#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_BuildCost.generated.h"

/**
 * GE_BuildCost - 건물 배치 시 골드 차감 이펙트
 * SetByCaller(Build.GoldCost)로 차감량을 동적으로 주입
 * GA_BuildPlace에서 실제 배치 셀 수 x 건물 cost를 계산 후 적용
 * 
 * [BP에서 설정]
 *  Duration Policy : Instant
 *  Modifiers
 *  Attribute         : SMPlayerAttributeSet.Gold
 *  Modifier Op       : Add
 *  Magnitude Type    : Set By Caller
 *  Set By Caller Tag : Build.GoldCost
 */
UCLASS()
class SAGOMAGIC_API UGE_BuildCost : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UGE_BuildCost();
};
