// SMSessionSubsystem.cpp


#include "SMSessionSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

USMSessionSubsystem::USMSessionSubsystem()
{
}

void USMSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IsValidSessionInterface())
	{
		InviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(
				this, &ThisClass::OnSessionUserInviteAcceptedInternal));
	}
}

void USMSessionSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(
			InviteAcceptedDelegateHandle);
	}
	Super::Deinitialize();
}

void USMSessionSubsystem::CreateSession(int32 MaxPlayer)
{
	if (IsValidSessionInterface() == false) return;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		DestroySession();
		return;
	}

	CreateSessionCompleteDelegateHandle =
		SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
			FOnCreateSessionCompleteDelegate::CreateUObject(
				this, &ThisClass::OnCreateSessionCompleteInternal));

	//세션 세팅 설정
	TSharedPtr<FOnlineSessionSettings> Settings = MakeShareable(new FOnlineSessionSettings());
	bool bIsDedicated = IsRunningDedicatedServer();
	Settings->NumPublicConnections = MaxPlayer;
	Settings->bIsDedicated = bIsDedicated;
	Settings->bUsesPresence = !bIsDedicated;
	Settings->bUseLobbiesIfAvailable = !bIsDedicated;
	Settings->bShouldAdvertise = true;
	Settings->bAllowJoinViaPresence = !bIsDedicated;
	Settings->bAllowJoinInProgress = false;
	Settings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(
		*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *Settings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
			CreateSessionCompleteDelegateHandle);
		OnCreateSessionComplete.Broadcast(false);
	}
}

void USMSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (IsValidSessionInterface() == false)
	{
		OnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &ThisClass::OnJoinSessionCompleteInternal));

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void USMSessionSubsystem::DestroySession()
{
	if (IsValidSessionInterface() == false)
	{
		OnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(
			this, &ThisClass::OnDestroySessionCompleteInternal));

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		OnDestroySessionComplete.Broadcast(false);
	}
}

void USMSessionSubsystem::OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegateHandle);
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void USMSessionSubsystem::OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegateHandle);
	OnJoinSessionComplete.Broadcast(Result);
}

void USMSessionSubsystem::OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(
		DestroySessionCompleteDelegateHandle);
	OnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void USMSessionSubsystem::OnSessionUserInviteAcceptedInternal(bool bWasSuccessful, int32 LocalUserNum,
                                                              FUniqueNetIdPtr UserId,
                                                              const FOnlineSessionSearchResult& InviteResult)
{
	if (bWasSuccessful == true)
	{
		JoinSession(InviteResult);
	}
}

bool USMSessionSubsystem::IsValidSessionInterface()
{
	if (SessionInterface.IsValid() == false)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			SessionInterface = Subsystem->GetSessionInterface();
		}
	}
	return SessionInterface.IsValid();
}
