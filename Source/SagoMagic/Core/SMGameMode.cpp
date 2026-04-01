#include "SMGameMode.h"

#include "SagoMagicPlayerController.h"

ASMGameMode::ASMGameMode()
{
}

void ASMGameMode::OnPostLogin(AController* NewPlayer)
{
    Super::OnPostLogin(NewPlayer);

    //TODO 은서 : ASMPlayerController가 생기면 교체가 필요(#include도 변경필요)
    ASagoMagicPlayerController* PC = Cast<ASagoMagicPlayerController>(NewPlayer);
    if (IsValid(PC))
    {
        AllPlayerController.Add(PC);
    }
}

void ASMGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
    ASagoMagicPlayerController* PC = Cast<ASagoMagicPlayerController>(Exiting);
    if (IsValid(PC))
    {
        AllPlayerController.Remove(PC);
    }
}
