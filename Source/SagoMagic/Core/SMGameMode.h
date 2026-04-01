#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

class ASagoMagicPlayerController;

/** Wave 진행 총괄, 서버 권위 */
UCLASS()
class SAGOMAGIC_API ASMGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    ASMGameMode();

    /** 로그인 */
    virtual void OnPostLogin(AController* NewPlayer) override;
    /** 로그아웃 */
    virtual void Logout(AController* Exiting) override;

protected:
    //TODO 은서 : SMPlayerController 생성 시 교체 되어야함 (Forward include도 제외)
    TArray<TObjectPtr<ASagoMagicPlayerController>> AllPlayerController;
};
