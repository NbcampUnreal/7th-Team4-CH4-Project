#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SMGameMode.generated.h"

class ASMBaseCampActor;
class ASMPlayerController;
class USMStateMachine;

/**
 * 서버 전용 - 플레이어 관리 + StateMachine 소유
*/
UCLASS()
class SAGOMAGIC_API ASMGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASMGameMode();
	/** 씸리스 트래블로 입장한 플레이어를 AllPlayerController에 등록 */
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	/** 퇴장한 플레이어를 AllPlayerController에서 제거 */
	virtual void Logout(AController* Exiting) override;

	/** 게임 시작 진입점 */
	virtual void BeginPlay() override;
	/** 매 프레임 StateMachine에 Tick 위임  */
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	/** 플레이어 사망시 호출되는 함수 */
	void OnPlayerDead(ASMPlayerController* InPlayerController);
	
	/** 플레이어가 L_Play에 도착하면 클라->서버로 호출되는 함수 */
	void OnPlayerReady(ASMPlayerController* InPlayerController);
	
    FSimpleDelegate OnWaveCleared;
	
	/** 게임 결과를 모든 클라이언트에 전달 */
	void BroadcastGameResult(bool bIsVictory);
	
	/** 베이스캠프 파괴 시 호출 */
	void OnBaseCampDestroyed();
	
	void RegisterBaseCamp(ASMBaseCampActor* InBaseCamp);
	ASMBaseCampActor* GetBaseCamp() const {return CachedBaseCamp;}
	
private:
	// 관전모드 진입
	void EnterSpectatorMode(TWeakObjectPtr<ASMPlayerController> InPlayerController);
	
	// 리스폰
	void RespawnPlayer(TWeakObjectPtr<ASMPlayerController> InPlayerController);
	
	/** MaxPlayer만큼 player가 모이면 게임 스타트*/
	void TryStartGame();
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

	int32 MaxPlayers = 2;

	// 관전 모드 대기 시간과 리스폰 타이머를 플레이어 별로 저장하기 위한 Map
	TMap<TObjectPtr<ASMPlayerController>, FTimerHandle> SpectatorTimerMap;
	TMap<TObjectPtr<ASMPlayerController>, FTimerHandle> RespawnTimerMap;
	
	/** 결과창을 보여준 후 로비로 강제 이동시킬 대기 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result")
	float ReturnToLobbyDelay = 10.0f;
	/** 다 같이 이동할 로비 맵 이름 */
	UPROPERTY(EditDefaultsOnly, Category = "Game Result")
	FString LobbyMapName = TEXT("L_Lobby");

	/** 로비 이동용 타이머 핸들 */
	FTimerHandle ReturnToLobbyTimerHandle;

	/** 실제로 로비로 서버 트래블을 실행하는 함수 */
	void ServerTravelToLobby();
	
	UPROPERTY()
	TObjectPtr<ASMBaseCampActor> CachedBaseCamp;
};
