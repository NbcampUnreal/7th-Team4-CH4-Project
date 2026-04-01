#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "AbilitySystemInterface.h"
#include "SMGameState.generated.h"

/** 상태 복제, Wave 전용 AbilitySystem */
UCLASS()
class SAGOMAGIC_API ASMGameState : public AGameState, public IAbilitySystemInterface
{
    GENERATED_BODY()
public:
    /** 생성자 */
    ASMGameState();

    /** AbilitySystemInterface 사용을 위한 함수*/
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    /** 어떤 변수를 복제할 것인가? */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    /** 클라이언트까지 네트워크 정보 수신이 끝난 시점에 호출 */
    virtual void PostNetInit() override;

};
