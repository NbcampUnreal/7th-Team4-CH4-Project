// SMCheckGameMode.cpp


#include "SMCheckGameMode.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"

ASMCheckGameMode::ASMCheckGameMode()
{
	// 로비 목록 등록 (포트, 상태파일 경로, 최대 인원)
	// EC2 배포 시 StatusFilePath를 실제 경로에 맞게 수정
	FLobbyInfo Lobby1;
	Lobby1.Port = 7778;
	Lobby1.StatusFilePath = TEXT("C:\\lobby_7778.txt");
	Lobby1.MaxPlayers = 4;
	LobbyList.Add(Lobby1);

	FLobbyInfo Lobby2;
	Lobby2.Port = 7779;
	Lobby2.StatusFilePath = TEXT("C:\\lobby_7779.txt");
	Lobby2.MaxPlayers = 4;
	LobbyList.Add(Lobby2);
}

void ASMCheckGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// serverip 커맨드라인 파싱은 InitGame에서 한 번만 수행
	// 없으면 기본값 127.0.0.1 유지
	FParse::Value(FCommandLine::Get(), TEXT("serverip="), ServerIP);
	UE_LOG(LogTemp, Log, TEXT("[L_Check] 서버 시작 - ServerIP: %s"), *ServerIP);
}

void ASMCheckGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	FString PlayerIP = NewPlayer->GetNetConnection() ? NewPlayer->GetNetConnection()->RemoteAddressToString() : TEXT("Unknown");
	UE_LOG(LogTemp, Log, TEXT("[L_Check] 플레이어 접속 - IP: %s"), *PlayerIP);
	
	RoutePlayerToLobby(NewPlayer);
}

int32 ASMCheckGameMode::ReadLobbyPlayerCount(const FString& FilePath)
{
	FString Content;
	if (FFileHelper::LoadFileToString(Content, *FilePath))
	{
		return FCString::Atoi(*Content);
	}
	return 0; // 파일 없으면 빈 방으로 간주
}

void ASMCheckGameMode::RoutePlayerToLobby(APlayerController* PC)
{
	if (IsValid(PC) == false) return;

	for (const FLobbyInfo& Lobby : LobbyList)
	{
		int32 PlayerCount = ReadLobbyPlayerCount(Lobby.StatusFilePath);
		
		UE_LOG(LogTemp, Log, TEXT("[L_Check] 로비 포트 %d 현재 인원: %d"), Lobby.Port, PlayerCount);
		
		if (PlayerCount == 0)
		{
			FString URL = FString::Printf(TEXT("%s:%d"), *ServerIP, Lobby.Port);
			
			UE_LOG(LogTemp, Log, TEXT("[L_Check] → %s 로 라우팅"), *URL);
			
			PC->ClientTravel(URL, TRAVEL_Absolute);
			return;
		}
	}

	// 모든 방이 꽉 참 → 타이틀로 복귀 (LobbyFull 파라미터로 이유 전달)
	UE_LOG(LogTemp, Warning, TEXT("[L_Check] 모든 로비 만석 → 타이틀로 복귀"));
	
	PC->ClientTravel(TEXT("/Game/SagoMagic/Maps/L_Title?LobbyFull=1"), TRAVEL_Absolute);
}
