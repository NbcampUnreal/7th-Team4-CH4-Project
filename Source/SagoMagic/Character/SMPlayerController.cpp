// SMPlayerController.cpp


#include "SMPlayerController.h"

void ASMPlayerController::BeginPlay()
{
    Super::BeginPlay();

    SetShowMouseCursor(true);

    FInputModeGameOnly InputMode;

    // 마우스 가두기
    InputMode.SetConsumeCaptureMouseDown(false);

    SetInputMode(InputMode);
}
