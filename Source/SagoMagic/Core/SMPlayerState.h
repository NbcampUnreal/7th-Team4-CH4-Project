// SMPlayerState.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GAS/SMAbilitySystemComponent.h"
#include "SMPlayerState.generated.h"

class USMItemDefinition;
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
	
	FORCEINLINE USMInventoryComponent* GetInventoryComponent() const { return SMInventoryComponent; }

protected:
	/** ASC */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<USMAbilitySystemComponent> SMAbilitySystemComponent;
	
	/** Inventory Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<USMInventoryComponent> SMInventoryComponent;
	
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
	
	//================================
	// 캐릭터 커스터마이징
	//================================
public:
	//Getter
	
	int32 GetSelectedWeaponIndex() const {return SelectedWeaponIndex;}
	int32 GetSelectedMaterialIndex() const {return SelectedMaterialIndex;}
	
	//Setter (ServerOnly)
	
	void SetSelectedWeaponIndex(int32 NewIndex);
	void SetSelectedMaterialIndex(int32 NewIndex);
	
protected:
	//서버에서 인덱스 변화 적용시 다른 클라이언트로 자동 replicate
	
	UFUNCTION()
	void OnRep_SelectedWeaponIndex();
	
	UFUNCTION()
	void OnRep_SelectedMaterialIndex();
	
	virtual void OnRep_PlayerName() override;
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_SelectedWeaponIndex)
	int32 SelectedWeaponIndex = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_SelectedMaterialIndex)
	int32 SelectedMaterialIndex = 0;
	
	//================================
	// 캐릭터 스킬 선택
	//================================
public:
	const TSoftObjectPtr<USMItemDefinition>& GetSelectedLobbySkillDef() const {return SelectedLobbySkillDef; }
	void SetSelectedLobbySkillDef(const TSoftObjectPtr<USMItemDefinition>& InDef);
private:
	UPROPERTY(Replicated)
	TSoftObjectPtr<USMItemDefinition> SelectedLobbySkillDef;
};
