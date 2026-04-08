// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMSessionSubsystem;
class USMTitleWidget;
class USMLobbyWidget;
class USMInventoryRootWidget;
class UInputAction;
class UInputMappingContext;
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
	virtual void SetupInputComponent() override;
	
	//================================
	// 네트워크 이동 기능
	//================================
public:
	
	/** ASMGameMode의 HandleSeamlessTravelPlayer에서 호출
	 *  Server에서 Client PC에게 알려줌
	 */
	UFUNCTION(Client, Reliable)
	void ClientRPCArrivedAtGameLevel();
	
	/** ServerRPC - 클라이언트 DataAsset 로드 완료 알림 */
	UFUNCTION(Server, Reliable)
	void ServerNotifyClientLoadComplete();

	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCSetReady(bool bReady);
	
	/** ServerRPC - USMLobbyWidget에서 호출 */
	UFUNCTION(Server, Reliable)
	void ServerRPCRequestStartGame();

	/** ServerRPC - 인벤토리 아이템 이동 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCMoveInventoryItem(
		const FGuid& InItemInstanceId,
		const FGuid& InTargetContainerId,
		int32 InGridX,
		int32 InGridY,
		ESMGridRotation InRotation);

	/** ServerRPC - 인벤토리 아이템 드랍 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCDropInventoryItem(const FGuid& InItemInstanceId);

	/** ServerRPC - 인벤토리 아이템 삭제 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCRemoveInventoryItem(const FGuid& InItemInstanceId);

	/** ServerRPC - 스킬 내부 장착 아이템 해제 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCDetachEmbeddedItem(const FGuid& InItemInstanceId);

public:
	/** 로컬 플레이어 인벤토리 UI 토글 */
	UFUNCTION(BlueprintCallable, Category="UI|Inventory")
	void ToggleInventory();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowDeathUI();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_HideDeathUI();
	
private:
	/** 컨트롤러 입력 매핑 컨텍스트 적용 */
	void ApplyControllerMappingContext();

	/** 현재 드래그 중인 인벤토리 아이템 회전 */
	void RotateDraggedInventoryItem();

	/** 컨트롤러 전용 입력 매핑 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputMappingContext> ControllerMappingContext;

	/** 인벤토리 토글 입력 액션 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	/** 인벤토리 드래그 회전 입력 액션 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> RotateInventoryItemAction;

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
