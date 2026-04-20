//SMLobbyGameMode.cpp


#include "SMLobbyGameMode.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSubsystem.h"
#include "SMLobbyGameState.h"
#include "SMPlayerSlotInfo.h"
#include "Core/SMPlayerState.h"
#include "SMSessionSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/CommandLine.h"

ASMLobbyGameMode::ASMLobbyGameMode()
{
	PlayerStateClass = ASMPlayerState::StaticClass();
	GameStateClass = ASMLobbyGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void ASMLobbyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	FParse::Value(FCommandLine::Get(), TEXT("port="), MyPort);
	FParse::Value(FCommandLine::Get(), TEXT("statuspath="), StatusFilePath);

	WriteStatusFile(0);
	UE_LOG(LogTemp, Log, TEXT("[L_Lobby] 서버 시작 - 포트: %d | 상태파일: %s"), MyPort, *StatusFilePath);
	
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		UE_LOG(LogTemp, Log, TEXT("[L_Lobby] OnlineSubsystem: %s"), *OSS->GetSubsystemName().ToString());
	}
}

void ASMLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	USMSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (IsValid(SessionSubsystem))
	{
		SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(this, &ThisClass::HandleCreateSessionComplete);
		SessionSubsystem->OnCreateSessionComplete.AddDynamic(this, &ThisClass::HandleCreateSessionComplete);
	}

	if (IsRunningDedicatedServer())
	{
		RequestLobbySession();
	}
}

void ASMLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerList.Add(NewPlayer);
	
	UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] 플레이어 접속 - 현재 인원: %d/%d"), MyPort, PlayerList.Num(), MaxPlayers);
	
	//첫 번째 접속자 처리 — PlayerState 유효 여부와 무관하게 항상 실행
	if (PlayerList.Num() == 1)
	{
		HostController = NewPlayer;

		UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] 첫 번째 접속자 → 방장 지정"), MyPort);

		if (!IsRunningDedicatedServer())
		{
			RequestLobbySession();
		}
	}

	//상태 파일 업데이트
	WriteStatusFile(PlayerList.Num());

	ASMPlayerState* NewPlayerState = GetSMPlayerState(NewPlayer);
	if (IsValid(NewPlayerState) == false) return;

	//첫 번째 접속자를 방장으로 지정
	//bIsHost는 Replicated이므로 클라이언트에 자동 전파
	NewPlayerState->bIsHost = PlayerList.Num() == 1;
	NewPlayerState->bIsReady = false;
	
	//Steam 닉네임 가져오기, 실패 시 "Player N 설정"
	FString NewPlayerName = FString::Printf(TEXT("Player %d"),PlayerList.Num());
	
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
		if (IdentityInterface.IsValid() == true)
		{
			//NewPlayer의 UniqueNetId로 닉네임 조회
			FUniqueNetIdRepl UniqueNetId = NewPlayer->GetPlayerState<APlayerState>()->GetUniqueId();
			if (UniqueNetId.IsValid() == true)
			{
				FString SteamName = IdentityInterface->GetPlayerNickname(*UniqueNetId);
				if (SteamName.IsEmpty() == false)
				{
					NewPlayerName = SteamName;
				}
			}
		}
	}
	NewPlayerState->SetPlayerName(NewPlayerName);
	
	UpdateLobbyState();
}

void ASMLobbyGameMode::Logout(AController* ExitingController)
{
	APlayerController* PC = Cast<APlayerController>(ExitingController);
	if (IsValid(PC) == false) return;

	PlayerList.Remove(PC);
	
	UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] 플레이어 퇴장 - 현재 인원: %d/%d"), MyPort, PlayerList.Num(), MaxPlayers);
	
	if (PC == HostController)
	{
		AssignNewHost();
	}
	
	// 마지막 플레이어 퇴장시 Steam 세션 삭제
	if (PlayerList.IsEmpty() == true)
	{
		if (!IsRunningDedicatedServer())
		{
			bSessionCreateRequested = false;
			USMSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
			if (IsValid(SessionSubsystem) == true)
			{
				SessionSubsystem->DestroySession();
				UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] 인원 0 → Steam 세션 삭제 요청"), MyPort);
			}
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] 인원 0이지만 Dedicated 서버 세션은 유지합니다."), MyPort);
		}
	}

	// 상태 파일 업데이트
	WriteStatusFile(PlayerList.Num());
	
	UpdateLobbyState();

	Super::Logout(ExitingController);
}

void ASMLobbyGameMode::SetPlayerReady(APlayerController* PC, bool bReady)
{
	ASMPlayerState* PS = GetSMPlayerState(PC);
	if (IsValid(PS) == false) return;
	PS->bIsReady = bReady;

	UpdateLobbyState();
}

void ASMLobbyGameMode::TryStartGame()
{
	if (PlayerList.IsEmpty() == true) return;
	if (IsAllReady() == false) return;

	FString MaxPlayer = FString::Printf(TEXT("?MaxPlayers=%d"),PlayerList.Num());
	FString URL = PlayMapRoot + MaxPlayer;
	GetWorld()->ServerTravel(URL);
}

bool ASMLobbyGameMode::IsAllReady() const
{
	if (PlayerList.IsEmpty() == true) return false;

	for (APlayerController* PC : PlayerList)
	{
		ASMPlayerState* PS = GetSMPlayerState(PC);
		if (IsValid(PS) == false) return false;

		if (PS->GetIsHost()) continue;

		if (PS->GetIsReady() == false) return false;
	}

	return true;
}

void ASMLobbyGameMode::AssignNewHost()
{
	HostController = nullptr;

	if (PlayerList.IsEmpty() == true) return;

	HostController = PlayerList[0];

	ASMPlayerState* PS = GetSMPlayerState(HostController);
	if (IsValid(PS) == true)
	{
		PS->bIsHost = true;
	}
}

void ASMLobbyGameMode::UpdateLobbyState()
{
	ASMLobbyGameState* GS = GetLobbyGameState();
	if (IsValid(GS) == false) return;

	TArray<FSMPlayerSlotInfo> PlayerSlots;

	for (APlayerController* PC : PlayerList)
	{
		ASMPlayerState* PS = GetSMPlayerState(PC);
		if (IsValid(PS) == false) continue;

		FSMPlayerSlotInfo SlotInfo;
		SlotInfo.PlayerName = PS->GetPlayerName();
		SlotInfo.bIsReady = PS->GetIsReady();
		SlotInfo.bIsHost = PS->GetIsHost();
		PlayerSlots.Add(SlotInfo);
	}

	GS->UpdatePlayerSlots(PlayerSlots);
}

ASMPlayerState* ASMLobbyGameMode::GetSMPlayerState(APlayerController* PC) const
{
	if (IsValid(PC) == false) return nullptr;
	return PC->GetPlayerState<ASMPlayerState>();
}

ASMLobbyGameState* ASMLobbyGameMode::GetLobbyGameState() const
{
	return GetGameState<ASMLobbyGameState>();
}

const TSoftObjectPtr<USMItemDefinition>& ASMLobbyGameMode::GetPresetSkillDefinition(int32 Index) const
{
	static TSoftObjectPtr<USMItemDefinition> InValidDefinition;
	if (PresetSkillDefinitions.IsValidIndex(Index) == false) return InValidDefinition;
	
	return PresetSkillDefinitions[Index];
}

void ASMLobbyGameMode::WriteStatusFile(int32 PlayerCount)
{
	if (StatusFilePath.IsEmpty() == true) return;

	FString Content = FString::FromInt(PlayerCount);
	FFileHelper::SaveStringToFile(Content, *StatusFilePath);
}

USMSessionSubsystem* ASMLobbyGameMode::GetSessionSubsystem() const
{
	UGameInstance* GI = GetGameInstance();
	if (IsValid(GI) == false) return nullptr;
	
	return GI->GetSubsystem<USMSessionSubsystem>();
}

void ASMLobbyGameMode::RequestLobbySession()
{
	if (bSessionCreateRequested)
	{
		UE_LOG(LogTemp, Log, TEXT("[L_Lobby:%d] Steam 세션 생성 요청은 이미 처리되었습니다."), MyPort);
		return;
	}

	USMSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (IsValid(SessionSubsystem) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[L_Lobby:%d] SessionSubsystem이 없어 Steam 세션을 생성할 수 없습니다."), MyPort);
		return;
	}

	bSessionCreateRequested = true;
	SessionSubsystem->CreateSession(MaxPlayers);

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[L_Lobby:%d] Steam 세션 생성 요청 (MaxPlayers: %d, Dedicated: %s)"),
		MyPort,
		MaxPlayers,
		IsRunningDedicatedServer() ? TEXT("true") : TEXT("false"));
}

void ASMLobbyGameMode::HandleCreateSessionComplete(bool bWasSuccessful)
{
	UE_LOG(
		LogTemp,
		Log,
		TEXT("[L_Lobby:%d] Steam 세션 생성 콜백 - 성공: %s"),
		MyPort,
		bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (!bWasSuccessful)
	{
		bSessionCreateRequested = false;
	}
}
