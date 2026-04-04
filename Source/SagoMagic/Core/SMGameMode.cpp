#include "SMGameMode.h"
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

    if (USMWaveManagerSubsystem* WM = GetWorld()->GetSubsystem<USMWaveManagerSubsystem>())
        WM->SetWaveDataTable(WaveDataTable);

    StateMachine = NewObject<USMStateMachine>(this);
    StateMachine->Initialize(this);

}

void ASMGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (StateMachine)
        StateMachine->Tick(DeltaSeconds);
}
