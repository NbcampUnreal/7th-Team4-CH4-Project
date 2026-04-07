// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMSessionSubsystem;
class USMTitleWidget;
class USMLobbyWidget;
class USMInventoryRootWidget;
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
	
	/** ASMGameMode의 HandleSeamlessTravelPlayer에서 호출
	 *  Server에서 Client PC에게 알려줌
	 */
	UFUNCTION(Client, Reliable)
	void ClientRPCArrivedAtGameLevel();
	
	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCSetReady(bool bReady);
	
	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCRequestStartGame();

public:
	/** 로컬 플레이어 인벤토리 UI 토글 */
	UFUNCTION(BlueprintCallable, Category="UI|Inventory")
	void ToggleInventory();
	
private:
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USMLobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TObjectPtr<USMLobbyWidget> LobbyWidgetInstance;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<USMInventoryRootWidget> InventoryRootWidgetClass;

	UPROPERTY()
	TObjectPtr<USMInventoryRootWidget> InventoryRootWidgetInstance;
	
	UPROPERTY(EditDefaultsOnly,Category = "Server|Maps")
	FString LobbyMapName = TEXT("L_Lobby");

	void ShowLobbyWidget();

	/** 인벤토리 위젯 생성 및 초기화 */
	void InitializeInventoryWidget();

	/** 인벤토리 위젯 표시 */
	void ShowInventoryWidget();

	/** 인벤토리 위젯 숨김 */
	void HideInventoryWidget();

	/** 현재 인벤토리 위젯 표시 여부 */
	bool bIsInventoryVisible = false;
};
