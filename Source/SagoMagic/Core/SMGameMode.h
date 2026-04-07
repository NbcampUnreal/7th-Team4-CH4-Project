#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

class ASMPlayerController;
class USMStateMachine;

//TODO List
// L_Play에서 GameMode 세팅
// PlayerSTate에 있는 이름값
// Player1, Player2 찍히는지 확인
// bUseSeamlessTravel = true;
// void ASMGameMode::HandleSeamlessTravelPlayer(AController*& C)

/**
 * 서버 전용 - 플레이어 관리 + StateMachine 소유
*/
UCLASS()
class SAGOMAGIC_API ASMGameMode : public AGameMode
{
    GENERATED_BODY()
public:
    ASMGameMode();
    /** 씸리스 트래블로 입장한 플레이어를 ALlPlayerController에 등록 */
    virtual void HandleSeamlessTravelPlayer(AController*& C) override;
    /** 퇴장한 플레이어를 AllPlayerController에서 제거 */
    virtual void Logout(AController* Exiting) override;

    /** 게임 시작 진입점 */
    virtual void BeginPlay() override;
    /** 매 프레임 StateMachine에 Tick 위임  */
    virtual void Tick(float DeltaSeconds) override;
	
	/** 플레이어 사망시 호출되는 함수 */
	void OnPlayerDead(ASMPlayerController* InPlayerController);

    FSimpleDelegate OnWaveCleared;

	
protected:
    /** 로그인 한 플레이어 Controller 모음 */
    UPROPERTY()
    TArray<TObjectPtr<ASMPlayerController>> AllPlayerController;
	
	/** 관전 모드 진입 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Timer")
	float SpectatorTime = 3.0f;
	
	/** 부활 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Timer")
	float RespawnTime = 10.0f;
	
private:
    /** 게임 플로우를 담당하는 FSM - 서버에만 존재 */
    UPROPERTY()
    TObjectPtr<USMStateMachine> StateMachine;
	
private:
	// 관전모드 진입
	void EnterSpectatorMode(ASMPlayerController* InPlayerController);
	
	// 리스폰
	void RespawnPlayer(ASMPlayerController* InPlayerController);
	
};
