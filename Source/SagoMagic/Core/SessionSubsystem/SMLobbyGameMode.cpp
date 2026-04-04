//SMLobbyGameMode.cpp


#include "SMLobbyGameMode.h"

#include "SMLobbyGameState.h"
#include "SMPlayerSlotInfo.h"
#include "Core/SMPlayerState.h"

ASMLobbyGameMode::ASMLobbyGameMode()
{
	PlayerStateClass = ASMPlayerState::StaticClass();
	GameStateClass = ASMLobbyGameState::StaticClass();
	bUseSeamlessTravel = true;
}

void ASMLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerList.Add(NewPlayer);

	ASMPlayerState* NewPlayerState = GetSMPlayerState(NewPlayer);
	if (IsValid(NewPlayerState) == false) return;

	//첫 번째 접속자를 방장으로 지정
	//bIsHost는 Replicated이므로 클라이언트에 자동 전파
	NewPlayerState->bIsHost = PlayerList.Num() == 1;
	NewPlayerState->bIsReady = false;
	FString NewPlayerName = FString::Printf(TEXT("Player %d"),PlayerList.Num());
	NewPlayerState->SetPlayerName(NewPlayerName);

	if (PlayerList.Num() == 1)
	{
		HostController = NewPlayer;
	}

	UpdateLobbyState();
}

void ASMLobbyGameMode::Logout(AController* ExitingController)
{
	APlayerController* PC = Cast<APlayerController>(ExitingController);
	if (IsValid(PC) == false) return;

	PlayerList.Remove(PC);
	if (PC == HostController)
	{
		AssignNewHost();
	}

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
