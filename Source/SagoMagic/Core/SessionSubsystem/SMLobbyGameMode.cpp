//SMLobbyGameMode.cpp


#include "SMLobbyGameMode.h"

#include "SMLobbyGameState.h"
#include "SMPlayerSlotInfo.h"
#include "Character/SMPlayerController.h"
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
	FString NewPlayerName = FString::Printf(TEXT(""));
	NewPlayerState->SetPlayerName(NewPlayerName);
	
	ASMPlayerController* NewSMPlayerController = Cast<ASMPlayerController>(NewPlayer);
	if (IsValid(NewSMPlayerController) == false) return;
	
	NewSMPlayerController->ClientRPCSetNickName();
	
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

const TSoftObjectPtr<USMItemDefinition>& ASMLobbyGameMode::GetPresetSkillDefinition(int32 Index) const
{
	static TSoftObjectPtr<USMItemDefinition> InValidDefinition;
	if (PresetSkillDefinitions.IsValidIndex(Index) == false) return InValidDefinition;
	
	return PresetSkillDefinitions[Index];
}

FString ASMLobbyGameMode::GenerateUniqueName(const APlayerController* Requester, const FString& DesiredName) const
{
	// 요청자 본인을 제외한 현재 이름 목록 수집
	TSet<FString> UsedNames;
	for (const APlayerController* PC : PlayerList)
	{
		if (PC == Requester) continue;

		const APlayerState* PS = PC->GetPlayerState<APlayerState>();
		if (IsValid(PS) == false) continue;

		UsedNames.Add(PS->GetPlayerName());
	}

	if (UsedNames.Contains(DesiredName) == false)
	{
		return DesiredName;
	}

	int32 Suffix = 1;
	FString UniqueName;
	do
	{
		UniqueName = FString::Printf(TEXT("%s %d"), *DesiredName, Suffix++);
	} while (UsedNames.Contains(UniqueName));

	return UniqueName;
}
