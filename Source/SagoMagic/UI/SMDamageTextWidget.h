#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMDamageTextWidget.generated.h"

class UTextBlock;

UCLASS()
class SAGOMAGIC_API USMDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 데미지 -> 텍스트 업데이트 함수 */
	void SetDamageText(float DamageAmount);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Damage;
};