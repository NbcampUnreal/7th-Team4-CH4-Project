// ASMTitleGameMode.cpp


#include "SMTitleGameMode.h"
#include "Core/SMGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ASMTitleGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	
	// L_Check에서 "LobbyFull=1" 파라미터와 함께 복귀한 경우
	FString LobbyFull = UGameplayStatics::ParseOption(Options, TEXT("LobbyFull"));
	if (LobbyFull == TEXT("1"))
	{
		USMGameInstance* GI = Cast<USMGameInstance>(GetGameInstance());
		if (IsValid(GI) == true)
		{
			GI->bWasLobbyFull = true;
		}
	}
}
