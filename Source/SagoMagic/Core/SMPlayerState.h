// SMPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "SMPlayerState.generated.h"

class USMQuickSlotComponent;
class USMPlayerAttributeSet;
class USMInventoryComponent;

/**
 * 플레이어의 정보를 저장할 클래스
 * 
 * ASC와 AttributeSet 소유
 */
UCLASS()
class SAGOMAGIC_API ASMPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASMPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	FORCEINLINE USMAbilitySystemComponent* GetSMAbilitySystemComponent() const { return SMAbilitySystemComponent; }

	FORCEINLINE USMPlayerAttributeSet* GetAttributeSet() const { return AttributeSet; }
	
	FORCEINLINE USMQuickSlotComponent* GetQuickSlotComponent() const { return QuickSlotComp; }

protected:
	/** ASC */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USMAbilitySystemComponent> SMAbilitySystemComponent;
	
	/** Inventory Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USMInventoryComponent> SMInventoryComponent;
	
	/** 퀵슬롯 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuickSlot")
	TObjectPtr<USMQuickSlotComponent> QuickSlotComp;
	

	/** Player Attribute Set */
	UPROPERTY()
	TObjectPtr<USMPlayerAttributeSet> AttributeSet;

	//================================
	// 네트워크 복제 전달 기능
	//================================
public:
	virtual void CopyProperties(APlayerState* PlayerState) override;

	bool GetIsHost() const { return bIsHost; }
	bool GetIsReady() const { return bIsReady; }
	
	void ResetForRespawn();

	//ServerRPC -> USMLobbyWidget -> ASMPlayerController -> 여기로 연결
	//ASMPlayerController.ServerSetReady() 에서 LobbyGameMode.SetPlayerReady() 호출
	//LobbyGameMode가 직접 bIsReady 변경하므로 RPC 불필요

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	//서버(LobbyGameMode)가 직접 설정
	//클라이언트는 읽기만
	UPROPERTY(Replicated)
	bool bIsHost;

	/** 서버(LobbyGameMode.SetPlayerReady())가 직접 설정 */
	UPROPERTY(Replicated)
	bool bIsReady;

	//friend 선언으로 LobbyGameMode를 제외한 다른 class는 해당 값을 수정할 수 없게 설정
	friend class ASMLobbyGameMode;
};
