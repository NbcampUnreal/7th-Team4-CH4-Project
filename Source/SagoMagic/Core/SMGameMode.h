#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

class ASMPlayerController;

/**
 * Wave 진행 총괄, 서버 권위
*/
UCLASS()
class SAGOMAGIC_API ASMGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    ASMGameMode();

    virtual void OnPostLogin(AController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

protected:
    /** 로그인 한 플레이어 Controller 모음 */
    UPROPERTY()
    TArray<TObjectPtr<ASMPlayerController>> AllPlayerController;
};
