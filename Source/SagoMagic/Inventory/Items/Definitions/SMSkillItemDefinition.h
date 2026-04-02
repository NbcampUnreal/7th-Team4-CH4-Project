#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Definitions/SMItemDefinition.h"
#include "SMSkillItemDefinition.generated.h"


/**
 * 스킬 아이템 정의 DataAsset 클래스 정의 파일
 *
 * 포함 내용:
 * - 스킬 전용 정의 클래스
 *
 * 역할:
 * - 스킬 아이템 에셋 생성 및 타입 분리용 정의 제공
 * - 추후 스킬 전용 데이터 작성 시 이곳 활용
 */

/** 스킬 아이템 정의 DataAsset 클래스 */
UCLASS(BlueprintType)
class SAGOMAGIC_API USMSkillItemDefinition : public USMItemDefinition
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    USMSkillItemDefinition() = default;
};
