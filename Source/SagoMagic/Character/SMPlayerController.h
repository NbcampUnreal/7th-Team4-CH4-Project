// SMPlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "SMPlayerController.generated.h"

class USMCustomizeWidget;
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
	/** 클라이언트에서 도착하면 호출되는 함수 */
	virtual void PostSeamlessTravel() override;
	
	/** Server로 클라이언트 도착 RPC*/
	UFUNCTION(Server,Reliable)
	void LoginNotify();
	
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

	/** ServerRPC - 활성 퀵슬롯 변경 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCSetActiveQuickSlot(int32 InSlotIndex);

	/** ServerRPC - 스킬 퀵슬롯 장착 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipSkillToQuickSlot(const FGuid& InSkillInstanceId, int32 InSlotIndex);

	/** ServerRPC - 스킬 빈 퀵슬롯 자동 장착 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCEquipSkillToFirstAvailableQuickSlot(const FGuid& InSkillInstanceId);

	/** ServerRPC - 퀵슬롯 스킬 자동 해제 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCUnequipSkillFromQuickSlot(int32 InSlotIndex);

	/** ServerRPC - 퀵슬롯 스킬 지정 위치 해제 요청 */
	UFUNCTION(Server, Reliable)
	void ServerRPCUnequipSkillFromQuickSlotToMainInventory(int32 InSlotIndex, int32 InGridX, int32 InGridY,
	                                                       ESMGridRotation InRotation);

public:
	/** 로컬 플레이어 인벤토리 UI 토글 */
	UFUNCTION(BlueprintCallable, Category="UI|Inventory")
	void ToggleInventory();
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowDeathUI(float RespawnTime);
	
	UFUNCTION(Client, Reliable)
	void ClientRPC_HideDeathUI();
	
	/** 게임 결과 UI 표시용 */
	UFUNCTION(Client, Reliable)
	void ClientRPC_ShowGameResult(bool bIsVictory, float InReturnDelay);
	
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

	UPROPERTY()
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
	
	//================================
	// 캐릭터 커스터마이징
	//================================
public:
	/** ServerRPC - SMCustomizeWidget에서 호출. 선택 확정 시 서버에 인덱스 전달 */
	UFUNCTION(Server, Reliable)
	void ServerRPCSetCustomization(int32 WeaponIndex, int32 MaterialIndex);
	
	/** LobbyWidget에서 호출. LobbyWidget 숨김 + CustomizeWidget 생성 + InputMode UIOnly */
	void OpenCustomizeWidget();
	
	/** CustomizeWidget에서 호출. CustomizeWidget제거 + LobbyWidget 복원 + InputMode GameAndUI */
	void CloseCustomizeWidget();
	
private:
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<USMCustomizeWidget> CustomizeWidgetClass;
	
	UPROPERTY()
	TObjectPtr<USMCustomizeWidget> CustomizeWidgetInstance;
	
	//================================
	// 캐릭터 스킬 선택
	//================================
public:
	UFUNCTION(Server, Reliable)
	void ServerRPCSelectLobbySkill(int32 InSkillIndex);
	
	/** L_Play진입 시 선택 스킬을 인벤토리에 초기화(서버 전용) */
	void InitializeInventory();
	
};
