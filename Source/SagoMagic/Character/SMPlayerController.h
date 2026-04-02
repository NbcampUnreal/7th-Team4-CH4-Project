// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

/**
 * UI 조작 및 캐릭터에 빙의 후 조작할 컨트롤러
 * Controller는 항상 하나의 클라이언트와 서버만 갖고 있다.
 */
UCLASS()
class SAGOMAGIC_API ASMPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
