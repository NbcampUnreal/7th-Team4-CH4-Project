// SMFunctionLibrary.cpp

#include "SMFunctionLibrary.h"
#include "SagoMagic.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void USMFunctionLibrary::SMPrintString(const UObject* WorldContextObject, const FString& InString, float InTimeToDisplay, FColor InColor)
{
	if (IsValid(GEngine) && IsValid(WorldContextObject))
	{
		UWorld* World = WorldContextObject->GetWorld();
		if (!World) return;
		
		if (World->GetNetMode() == NM_Client || World->GetNetMode() == NM_ListenServer)
		{
			GEngine->AddOnScreenDebugMessage(-1, InTimeToDisplay, InColor, InString);
		}
		else
		{
			UE_LOG(LogSM, Log, TEXT("%s"), *InString);
		}
	}
}

FString USMFunctionLibrary::GetNetModeString(const UObject* WorldContextObject)
{
	return SMLogPrivate::GetNetModeString(WorldContextObject);
}

FString USMFunctionLibrary::GetRoleString(const AActor* InActor)
{
	FString RoleString = TEXT("None");

	if (IsValid(InActor))
	{
		FString LocalRoleString = UEnum::GetValueAsString(
			TEXT("Engine.ENetRole"),
			InActor->GetLocalRole());

		FString RemoteRoleString = UEnum::GetValueAsString(
			TEXT("Engine.ENetRole"),
			InActor->GetRemoteRole());

		RoleString = FString::Printf(TEXT("%s / %s"), *LocalRoleString, *RemoteRoleString);
	}

	return RoleString;
}
