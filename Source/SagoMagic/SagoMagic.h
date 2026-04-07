// SagoMagic.h

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
#pragma region NetLogging

DECLARE_LOG_CATEGORY_EXTERN(LogSM, Log, All);

namespace SMLogPrivate
{
	FORCEINLINE FString GetNetModeString(const UWorld* World)
	{
		if (!IsValid(World)) return TEXT("None");
		const ENetMode NetMode = World->GetNetMode();
		if (NetMode == ENetMode::NM_Client) return FString::Printf(TEXT("Client%02d"), UE::GetPlayInEditorID());
		return (NetMode == ENetMode::NM_Standalone) ? TEXT("StandAlone") : TEXT("Server");
	}
	FORCEINLINE FString GetNetModeString(const UObject* WorldContextObject)
	{
		return IsValid(WorldContextObject) ? GetNetModeString(WorldContextObject->GetWorld()) : TEXT("None");
	}
}

#define NETMODE_TCHAR(WorldContextObject) (*SMLogPrivate::GetNetModeString(WorldContextObject))
#define FUNCTION_TCHAR (ANSI_TO_TCHAR(__FUNCTION__))
#define SM_LOG(WorldContextObject, LogCategory, Verbosity, Format, ...) \
UE_LOG(LogCategory, Verbosity, TEXT("[%s] %s %s"), NETMODE_TCHAR(WorldContextObject), FUNCTION_TCHAR, *FString::Printf(Format, ##__VA_ARGS__))

#pragma endregion
