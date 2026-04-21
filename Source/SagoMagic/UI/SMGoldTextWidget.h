#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMGoldTextWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class SAGOMAGIC_API USMGoldTextWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetGoldText(float GoldDelta);
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlockGold;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageGold;
};
