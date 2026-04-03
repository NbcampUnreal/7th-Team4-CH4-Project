#include "SMHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/SMHUDManager.h"

void ASMHUD::BeginPlay()
{
	Super::BeginPlay();
	
	if (HUDManagerClass)
	{
		HUDManager = CreateWidget<USMHUDManager>(GetOwningPlayerController(), HUDManagerClass);
		if (HUDManager)
		{
			HUDManager->AddToViewport();
		}
	}
}
