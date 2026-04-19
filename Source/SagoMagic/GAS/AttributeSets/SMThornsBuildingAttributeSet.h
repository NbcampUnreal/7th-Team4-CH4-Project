#pragma once

#include "CoreMinimal.h"
#include "SMBuildingAttributeSet.h"
#include "SMThornsBuildingAttributeSet.generated.h"

/**
 * 때릴 때마다 역으로 데미지를 주는 fence용 AttributeSet
 */
UCLASS()
class SAGOMAGIC_API USMThornsBuildingAttributeSet : public USMBuildingAttributeSet
{
	GENERATED_BODY()
public:
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
