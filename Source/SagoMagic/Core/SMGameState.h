#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SMGameState.generated.h"

UENUM(BlueprintType)
enum class EGameState : uint8
{
    None UMETA(DisplayName = "None"),
    Build UMETA(DisplayName = "Build"),
    Combat UMETA(DisplayName = "Combat"),
    Result UMETA(DisplayName = "Result"),
};

// 클라이언트에서 페이즈 변경 감지용 (HUD, 이펙트 등)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameStateChanged, EGameState);

/**
 * 서버 -> 클라이언트 복제 담당
 * StateMachine(서버)이 페이즈를 바꾸면 CurrentState가 클라이언트에 자동 복제됨
*/
UCLASS()
class SAGOMAGIC_API ASMGameState : public AGameState
{
    GENERATED_BODY()
public:
    /** 생성자 */
    ASMGameState();

    /** 어떤 변수를 복제할 것인가? */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    /** 클라이언트까지 네트워크 정보 수신이 끝난 시점에 호출 */
    virtual void PostNetInit() override;

    /**StateMachine이 페이즈 바꿀 때 서버에서 호출*/
    void SetCurrentState(EGameState NewState);
    EGameState GetCurrentState() const {return CurrentState;}
    
    /** 서버 -> 모든 클라이언트에게 로드할 AssetID 목록 전달 */
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPreloadClientAssets(const TArray<FPrimaryAssetId>& AssetIds);
    
    //클라이언트 구독용
    FOnGameStateChanged OnGameStateChanged;
private:
    /** CurrentState 복제 완료 시 클라이언트에서 자동 호출 -> OnGameStateChanged 브로드캐스트 */
    UFUNCTION()
    void OnRep_CurrentState();

    /** 현재 게임 페이즈 - 서버에서 쓰고 클라이언트에 복제됨 */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentState)
    EGameState CurrentState = EGameState::None;
};


