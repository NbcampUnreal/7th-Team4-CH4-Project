#include "SMGameMode.h"

#include "SagoMagic.h"
#include "SMPlayerState.h"
#include "Character/SMPlayerController.h"
#include "Core/SMStateMachine.h"
#include "Wave/SMWaveManagerSubsystem.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ASMGameMode::ASMGameMode()
{
}

void ASMGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
    Super::HandleSeamlessTravelPlayer(C);
	//TODO 은서 : Timer를 걸어서, 시간안에 인원수 안들어오면 어떻게 할지 테스트
    ASMPlayerController* PC = Cast<ASMPlayerController>(C);
    if (IsValid(PC))
    {
        AllPlayerController.AddUnique(PC);
        PC->ClientRPCArrivedAtGameLevel();
    }
}

void ASMGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	ASMPlayerController* PC = Cast<ASMPlayerController>(Exiting);
	if (IsValid(PC))
	{
		AllPlayerController.Remove(PC);

		// 플레이어가 종료시 타이머 제거
		if (FTimerHandle* HandlePtr = SpectatorTimerMap.Find(PC))
		{
			GetWorldTimerManager().ClearTimer(*HandlePtr);
		}
		if (FTimerHandle* HandlePtr = RespawnTimerMap.Find(PC))
		{
			GetWorldTimerManager().ClearTimer(*HandlePtr);
		}

		SpectatorTimerMap.Remove(PC);
		RespawnTimerMap.Remove(PC);
	}
}

void ASMGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASMGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (StateMachine)
		StateMachine->Tick(DeltaSeconds);
}

void ASMGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	MaxPlayers = UGameplayStatics::GetIntOption(Options, TEXT("MaxPlayers"), 4);
	UE_LOG(LogTemp, Log, TEXT("[GameMode] MaxPlayers: %d"), MaxPlayers);
	StateMachine = NewObject<USMStateMachine>(this);
}

void ASMGameMode::OnPlayerDead(ASMPlayerController* InPlayerController)
{
	if (!InPlayerController) return;

	SM_LOG(this, LogSM, Error,
		TEXT("[GameMode] 플레이어 사망. 관전모드(%.1f) 및 부활(%.1f) 타이머 시작"), SpectatorTime, RespawnTime);

	// 사망 UI 표시 - RespawnTime을 포함해서 클라이언트에 전달
	InPlayerController->ClientRPC_ShowDeathUI(RespawnTime);
	
	TWeakObjectPtr<ASMPlayerController> WeakPC = InPlayerController;

	// 중복 타이머가 있다면 제거
	if (FTimerHandle* HandlePtr = SpectatorTimerMap.Find(InPlayerController))
	{
		GetWorldTimerManager().ClearTimer(*HandlePtr);
	}
	if (FTimerHandle* HandlePtr = RespawnTimerMap.Find(InPlayerController))
	{
		GetWorldTimerManager().ClearTimer(*HandlePtr);
	}

	FTimerDelegate SpectatorDelegate =
		FTimerDelegate::CreateUObject(this, &ThisClass::EnterSpectatorMode, WeakPC);
	GetWorldTimerManager().SetTimer(
		SpectatorTimerMap.FindOrAdd(InPlayerController),
		SpectatorDelegate,
		SpectatorTime,
		false);

	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::RespawnPlayer, WeakPC);
	GetWorldTimerManager().SetTimer(
		RespawnTimerMap.FindOrAdd(InPlayerController),
		RespawnDelegate,
		RespawnTime,
		false);
}

void ASMGameMode::OnPlayerReady(ASMPlayerController* InPlayerController)
{
	if (!InPlayerController) return;
	UE_LOG(LogTemp,Warning,TEXT("OnPlayerReady"));
	AllPlayerController.AddUnique(InPlayerController);
	TryStartGame();
}

void ASMGameMode::BroadcastGameResult(bool bIsVictory)
{
	SM_LOG(this, LogSM, Log, TEXT("[GameMode] 게임 결과 브로드캐스트 - %s"),
		bIsVictory ? TEXT("승리") : TEXT("패배"));

	// AllPlayerController에 등록된 모든 클라이언트에게 결과 전달
	for (ASMPlayerController* PC : AllPlayerController)
	{
		if (IsValid(PC))
		{
			PC->ClientRPC_ShowGameResult(bIsVictory, ReturnToLobbyDelay);
		}
	}
	// 서버에서 로비 이동 타이머 가동
	GetWorldTimerManager().SetTimer(
		ReturnToLobbyTimerHandle,
		this,
		&ThisClass::ServerTravelToLobby,
		ReturnToLobbyDelay,
		false
	);
}

void ASMGameMode::ServerTravelToLobby()
{
	SM_LOG(this, LogSM, Log, TEXT("[GameMode] 시간이 초과되어 모든 플레이어를 로비로 이동시킵니다."));
	
	GetWorld()->ServerTravel(LobbyMapName);
}

void ASMGameMode::OnBaseCampDestroyed()
{
	SM_LOG(this, LogSM, Error, TEXT("[GameMode] 베이스캠프 파괴 - 패배 처리"));

	// StateMachine이 살아있으면 Result 상태로 강제 전환
	if (StateMachine)
	{
		StateMachine->SetPendingVictory(false);
		StateMachine->ChangeState(EGameState::Result);
		// ChangeState -> SMResultState::Enter() -> BroadcastGameResult(false) 자동 호출
	}
}

void ASMGameMode::RegisterBaseCamp(ASMBaseCampActor* InBaseCamp)
{
	CachedBaseCamp = InBaseCamp;
}

void ASMGameMode::EnterSpectatorMode(TWeakObjectPtr<ASMPlayerController> InPlayerController)
{
	if (!InPlayerController.IsValid()) return;

	InPlayerController->UnPossess();

	// TODO: 추후 기획 확정 시 관전 처리 로직 작성
	// 임시로 비행 관전모드 진입
	// InPlayerController->ChangeState(NAME_Spectating);

	SM_LOG(this, LogSM, Error, TEXT("[GameMode] 관전 모드 진입"));
}

void ASMGameMode::RespawnPlayer(TWeakObjectPtr<ASMPlayerController> InPlayerController)
{
	if (!InPlayerController.IsValid()) return;

	ASMPlayerController* PC = InPlayerController.Get();
	
	// 관전 폰 제거
	if (APawn* CurrentPawn = PC->GetPawn())
	{
		CurrentPawn->Destroy();
	}
	PC->UnPossess();
	PC->ChangeState(NAME_Playing);

	if (ASMPlayerState* PS = InPlayerController->GetPlayerState<ASMPlayerState>())
	{
		PS->ResetForRespawn();
	}

	// 스폰 포인트 지정 후 리스폰
	AActor* SpawnPoint = ChoosePlayerStart(PC);
    if (!SpawnPoint)  
    {
       SM_LOG(this, LogSM, Warning, TEXT("[GameMode] 엔진이 스폰 포인트를 찾지 못해 강제 할당을 시도합니다."));
       for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
       {
          SpawnPoint = *It;
          break;
       }
    }
   
    if (SpawnPoint)  // 최종적으로 스폰 포인트가 확보되었는지 확인 후 부활
    {
        RestartPlayerAtPlayerStart(PC, SpawnPoint);
    }
    else
    {
        SM_LOG(this, LogSM, Error, TEXT("[GameMode] 맵에 PlayerStart가 아예 존재하지 않아 부활에 실패했습니다!"));
    }
	InPlayerController->ClientRPC_HideDeathUI();
	
	// 부활시 타이머 정리
	SpectatorTimerMap.Remove(PC);
	RespawnTimerMap.Remove(PC);

	SM_LOG(this, LogSM, Error, TEXT("[GameMode] %s 플레이어 부활"), *PC->GetName());
}

void ASMGameMode::TryStartGame()
{
	if (AllPlayerController.Num() >= MaxPlayers)
	{
		if (!StateMachine)
		{
			UE_LOG(LogTemp, Error, TEXT("[GameMode] TryStartGame - StateMachine nullptr! 무시"));
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] 모든 플레이어 도착 %d - 게임 시작"), MaxPlayers);
		StateMachine->Initialize(this);
	}
}
