#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SMHUD.generated.h"


UCLASS()
class SAGOMAGIC_API ASMHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class USMHUDManager> HUDManagerClass;
	
	UPROPERTY()
	class USMHUDManager* HUDManager;
};
