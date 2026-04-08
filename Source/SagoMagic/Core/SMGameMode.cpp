#include "SMGameMode.h"

#include "SagoMagic.h"
#include "SMPlayerState.h"
#include "Character/SMPlayerController.h"
#include "Core/SMStateMachine.h"
#include "Wave/SMWaveManagerSubsystem.h"

ASMGameMode::ASMGameMode()
{
}

void ASMGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);
    ASMPlayerController* PC = Cast<ASMPlayerController>(C);
    if (IsValid(PC))
    {
        AllPlayerController.Add(PC);
        PC->ClientRPCArrivedAtGameLevel();
        TryStartGame();
    }
}


void ASMGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    ASMPlayerController* PC = Cast<ASMPlayerController>(Exiting);
    if (IsValid(PC))
    {
        AllPlayerController.Remove(PC);
    }
}

void ASMGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    StateMachine = NewObject<USMStateMachine>(this);

}

void ASMGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (StateMachine)
        StateMachine->Tick(DeltaSeconds);
}

void ASMGameMode::OnPlayerDead(ASMPlayerController* InPlayerController)
{
	if (!InPlayerController) return;
	
	SM_LOG(this, LogSM, Error, TEXT("[GameMode] 플레이어 사망. 관전모드 및 부활 타이머 시작"));
	
	TWeakObjectPtr<ASMPlayerController> WeakPC = InPlayerController;
	
	FTimerHandle SpectatorTimerHandle;
	FTimerDelegate SpectatorDelegate = 
		FTimerDelegate::CreateUObject(this, &ThisClass::EnterSpectatorMode, WeakPC);
	GetWorldTimerManager().SetTimer(SpectatorTimerHandle, SpectatorDelegate, SpectatorTime, false);
	
	FTimerHandle RespawnTimerHandle;
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::RespawnPlayer, WeakPC);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnTime, false);
}

void ASMGameMode::EnterSpectatorMode(TWeakObjectPtr<ASMPlayerController> InPlayerController)
{
	if (!InPlayerController.IsValid()) return;
	
	InPlayerController->UnPossess();
	
	// TODO: 추후 기획 확정 시 관전 처리 로직 작성
	// 임시로 비행 관전모드 진입
	InPlayerController->ChangeState(NAME_Spectating);
	
	SM_LOG(this, LogSM, Error, TEXT("[GameMode] %f초 뒤 관전 모드 진입"), SpectatorTime);
}

void ASMGameMode::RespawnPlayer(TWeakObjectPtr<ASMPlayerController> InPlayerController)
{
	if (!InPlayerController.IsValid()) return;
	
	ASMPlayerController* PC = InPlayerController.Get();
	
	if (ASMPlayerState* PS = InPlayerController->GetPlayerState<ASMPlayerState>())
	{
		PS->ResetForRespawn();
	}
	
	// 스폰 포인트 지정 후 리스폰
	AActor* SpawnPoint = ChoosePlayerStart(PC);
	RestartPlayerAtPlayerStart(PC, SpawnPoint);
	
	InPlayerController->ClientRPC_HideDeathUI();
	
	SM_LOG(this, LogSM, Error, TEXT("[GameMode] %f초 뒤 부활"), RespawnTime);
}
void ASMGameMode::TryStartGame()
{
	if (AllPlayerController.Num() >= MaxPlayers)
	{
		UE_LOG(LogTemp,Warning, TEXT("[GameMode] 모든 플레이어 도착 %d - 게임 시작"),MaxPlayers);
		StateMachine->Initialize(this);
	}
}
