// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

/**
 *
 */
UCLASS()
class SAGOMAGIC_API ASMPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
};
