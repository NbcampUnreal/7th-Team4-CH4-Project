#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMGoldFloatingText.generated.h"

class UWidgetComponent;

UCLASS()
class SAGOMAGIC_API ASMGoldFloatingText : public AActor
{
	GENERATED_BODY()

public:
	ASMGoldFloatingText();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> GoldWidgetComp;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float LifeSpan = 1.0f;
	
	void SetGoldValue(float GoldDelta);
private:
	float PendingGold = 0.f;
};
