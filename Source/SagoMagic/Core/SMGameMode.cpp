#include "SMGameMode.h"
#include "Character/SMPlayerController.h"

ASMGameMode::ASMGameMode()
{
}

void ASMGameMode::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    ASMPlayerController* PC = Cast<ASMPlayerController>(NewPlayer);
    if (IsValid(PC))
    {
        AllPlayerController.Add(PC);
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
