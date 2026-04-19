// SMCheckGameMode.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SMCheckGameMode.generated.h"

USTRUCT()
struct FLobbyInfo
{
	GENERATED_BODY()
	int32 Port = 0;
	FString StatusFilePath;
	int32 MaxPlayers = 4;
};

/**
 * 유저가 접속을 시도하면 남아있는 로비를 확인하여
 * 유저가 비어있는 로비로 접속할 수 있도록 합니다.
 */
UCLASS()
class SAGOMAGIC_API ASMCheckGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASMCheckGameMode();
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:
	FString ServerIP = TEXT("127.0.0.1");
	TArray<FLobbyInfo> LobbyList;

	int32 ReadLobbyPlayerCount(const FString& FilePath);
	void RoutePlayerToLobby(APlayerController* PC);
};
