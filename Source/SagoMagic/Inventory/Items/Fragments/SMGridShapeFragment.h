#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMGridShapeFragment.generated.h"


/**
 * 아이템 모양 마스크 Fragment 정의 파일
 *
 * 포함 내용:
 * - 비트마스크 기반 모양 데이터
 *
 * 역할:
 * - 아이템 배치, 회전, 점유 셀 계산에 사용하는 모양 마스크 제공
 */

/** 아이템 모양 마스크 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMGridShapeFragment : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMGridShapeFragment() = default;

	/** 모양 마스크 Getter */
	const FSMGridMaskData& GetShapeMask() const
	{
		return ShapeMask;
	}

	/** 모양 마스크 Setter */
	void SetShapeMask(const FSMGridMaskData& InShapeMask)
	{
		ShapeMask = InShapeMask;
	}

public:
	/** 모양 마스크 데이터 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Grid Shape Fragment")
	FSMGridMaskData ShapeMask;
};
