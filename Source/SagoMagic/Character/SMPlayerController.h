// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SMPlayerController.generated.h"

class USMSessionSubsystem;
class USMMainWidget;
class USMLobbyWidget;
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
	
	//================================
	// 네트워크 이동 기능
	//================================
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// USMainWidget에서 호출
	void SetPendingServerAddress(const FString& Address);
	
	// ASMGameMode의 HandleSeamlessTravelPlayer에서 호출
	void OnArrivedAtGameLevel();
	
	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCSetReady(bool bReady);
	
	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCRequestStartGame();
	
private:
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USMMainWidget> MainWidgetClass;
	
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USMLobbyWidget> LobbyWidgetClass;
	
	FString PendingServerAddress = TEXT("127.0.0.1:7777");
	
	UPROPERTY(EditDefaultsOnly,Category = "Server|Maps")
	FString MainMapName = TEXT("L_Main");
	UPROPERTY(EditDefaultsOnly,Category = "Server|Maps")
	FString LobbyMapName = TEXT("L_Lobby");
	
	UPROPERTY()
	TObjectPtr<USMSessionSubsystem> SessionSubsystem;
	
	void ShowMainWidget();
	void ShowLobbyWidget();
	
	UFUNCTION()
	void OnCreateSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);
	
	void TravelToServer();
	
	void BindSessionDelegates();
	void UnbindSessionDelegates();
	
};
