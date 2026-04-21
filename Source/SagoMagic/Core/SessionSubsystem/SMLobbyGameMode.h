// SMLobbyGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SMLobbyGameMode.generated.h"


class USMItemDefinition;
class ASMPlayerState;
class ASMLobbyGameState;
/**
 *로비 전용 게임모드 입니다.
 *플레이어의 로그인, 레디, 게임시작을 처리합니다.
 */
UCLASS()
class SAGOMAGIC_API ASMLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASMLobbyGameMode();

	//로그인 후 처리
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* ExitingController) override;

	//ASMPlayerController.ServerRPCSetReady() RPC에서 호출할 예정
	void SetPlayerReady(APlayerController* PC, bool bReady);

	//ASMPlayerController.ServerRPCRequestStartGame() RPC에서 호출할 예정
	void TryStartGame();

	//로비 환경 변화시 로비 상태 업데이트
	void UpdateLobbyState();
	
protected:
	UPROPERTY()
	TArray<TObjectPtr<APlayerController>> PlayerList;

	UPROPERTY()
	TObjectPtr<APlayerController> HostController;
	
	UPROPERTY(EditDefaultsOnly, Category = "PlayMapRoot")
	FString PlayMapRoot = TEXT("/Game/SagoMagic/Maps/L_Play");
	
	//방장을 제외한 모든 플레이어 ready확인
	bool IsAllReady() const;
	//방장 이탈시 새로운 호스트 임명
	void AssignNewHost();

	ASMPlayerState* GetSMPlayerState(APlayerController* PC) const;
	ASMLobbyGameState* GetLobbyGameState() const;
	
	//================================
	// 캐릭터 스킬 선택
	//================================
public:
	const TSoftObjectPtr<USMItemDefinition>& GetPresetSkillDefinition(int32 index) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Preset")
	TArray<TSoftObjectPtr<USMItemDefinition>> PresetSkillDefinitions;
	
	//================================
	// 닉네임 설정
	//================================
public:
	/** 기존 플레이어 이름과 중복되지 않는 유니크한 이름 반환 (요청자 본인 제외) */
	FString GenerateUniqueName(const APlayerController* Requester, const FString& DesiredName) const;
};
