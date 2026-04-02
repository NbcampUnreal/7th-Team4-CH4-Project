#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "SMGemItemDefinition.generated.h"


/**
 * 젬 아이템 정의 DataAsset 클래스 정의 파일
 *
 * 포함 내용:
 * - 젬 전용 정의 클래스
 *
 * 역할:
 * - 젬 아이템 에셋 생성 및 타입 분리용 정의 제공
 * - 추후 젬 전용 데이터 작성시 이곳 활용
 */

/** 젬 아이템 정의 DataAsset 클래스 */
UCLASS(BlueprintType)
class SAGOMAGIC_API USMGemItemDefinition : public USMItemDefinition
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMGemItemDefinition() = default;
};
