#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SMItemFragment.generated.h"


/**
 * 공통 아이템 Fragment 베이스 클래스 정의 파일
 *
 * 포함 내용:
 * - 모든 아이템 Fragment의 공통 부모 클래스
 *
 * 역할:
 * - ItemDefinition이 소유하는 기능 조각의 공통 타입 제공
 */

/** 공통 아이템 Fragment 베이스 클래스 */
UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMItemFragment : public UObject
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMItemFragment() = default;
};
