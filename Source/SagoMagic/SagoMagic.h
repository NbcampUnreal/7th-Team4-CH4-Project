// SagoMagic.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/** Main log category used across the project */
#pragma region NetLogging

DECLARE_LOG_CATEGORY_EXTERN(LogSM, Log, All);

#define NETMODE_TCHAR ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("Client%02d"), UE::GetPlayInEditorID()) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("StandAlone") : TEXT("Server")))
#define FUNCTION_TCHAR (ANSI_TO_TCHAR(__FUNCTION__))
#define SM_LOG(LogCategory, Verbosity, Format, ...) UE_LOG(LogCategory, Verbosity, TEXT("[%s] %s %s"), NETMODE_TCHAR, FUNCTION_TCHAR, *FString::Printf(Format, ##__VA_ARGS__))

#pragma endregion 

UCLASS()
class SAGOMAGIC_API USMFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "SM|Debug", meta = (WorldContext = "WorldContextObject"))
	static void SMPrintString(const UObject* WorldContextObject, const FString& InString, float InTimeToDisplay = 3.0f, FColor InColor = FColor::Cyan)
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
	
	UFUNCTION(BlueprintCallable, Category = "SM|Debug", meta = (WorldContext = "WorldContextObject"))
	static FString GetNetModeString(const UObject* WorldContextObject)
	{
		FString NetModeString = TEXT("None");
		
		if (IsValid(WorldContextObject))
		{
			UWorld* World = WorldContextObject->GetWorld();
			if (!World) return NetModeString;
			
			ENetMode NetMode = World->GetNetMode();
			
			if (NetMode == NM_Client)
			{
				NetModeString = TEXT("Client");
			}
			else
			{
				if (NetMode == NM_Standalone)
				{
					NetModeString = TEXT("StandAlone");
				}
				else
				{
					NetModeString = TEXT("Server");
				}
			}
		}
		
		return NetModeString;
	}
	
	UFUNCTION(BlueprintCallable, Category = "SM|Debug")
	static FString GetRoleString(const AActor* InActor)
	{
		FString RoleString = TEXT("None");
		
		if (IsValid(InActor))
		{
			FString LocalRoleString = UEnum::GetValueAsString(
				TEXT("Engine.ENetRole"), InActor->GetLocalRole());
			
			FString RemoteRoleString = UEnum::GetValueAsString(
				TEXT("Engine.ENetRole"), InActor->GetRemoteRole());
			
			RoleString = FString::Printf(TEXT("%s / %s"), *LocalRoleString, *RemoteRoleString);
		}
		
		return RoleString;
	}
};
