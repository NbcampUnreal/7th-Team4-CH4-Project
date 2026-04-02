//SMLobbyGameMode.cpp


#include "SMLobbyGameMode.h"

#include "Core/SMPlayerState.h"

ASMLobbyGameMode::ASMLobbyGameMode()
{
    PlayerStateClass = ASMPlayerState::StaticClass();
    //GameStateClass = ASMLobbyGameState::StaticClass();
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
    //PS->bIsHost = PlayerList.Num() == 1;
    //PS-> bISReady = false;

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

    //PS->bIsReady = bReady;

    UpdateLobbyState();
}

void ASMLobbyGameMode::TryStartGame()
{
    if (PlayerList.IsEmpty() == true) return;
    if (IsAllReady() == false) return;

    //TODO: GameInstance에 로비정보 저장? --> 회의 이후 결정

    GetWorld()->ServerTravel(TEXT("/Game/Maps/L_Play?listen"));

}

bool ASMLobbyGameMode::IsAllReady() const
{
    if (PlayerList.IsEmpty() == true) return false;

    for (APlayerController* PC : PlayerList)
    {
        ASMPlayerState* PS = GetSMPlayerState(PC);
        if (IsValid(PS) == false) return false;

        //TODO:PlayerState에서 호스트 판별 및 Ready 상태 체크
        //if(PS->GetIsHost()) continue;
        //if(PS->GetIsReady()) return false;
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
        //TODO: PlayerState에 host 설정
        //PS->bIsHost = true;
    }
}

void ASMLobbyGameMode::UpdateLobbyState()
{
    //TODO:GameState 생성후 GameState를 통해 로비 정보 업데이트
}

ASMPlayerState* ASMLobbyGameMode::GetSMPlayerState(APlayerController* PC) const
{
    if (IsValid(PC) == false) return nullptr;
    return PC->GetPlayerState<ASMPlayerState>();
}

