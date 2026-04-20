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
	
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem && Subsystem->GetSubsystemName().ToString() == "NULL")
	{
		UE_LOG(LogTemp, Warning, TEXT("[SMSessionSubsystem] Steam 미연결 - NULL 서브시스템 사용 중"));
	}

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

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	const FName SubsystemName = Subsystem ? Subsystem->GetSubsystemName() : NAME_None;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Session] 기존 GameSession이 남아 있어 재생성 전에 DestroySession을 요청합니다."));
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
	Settings->bAllowJoinInProgress = true;
	Settings->BuildUniqueId = 1; 
	Settings->bIsLANMatch = SubsystemName == "NULL";
	
	bool bSuccess = false;

	UE_LOG(
		LogTemp,
		Log,
		TEXT("[Session] CreateSession 요청 - OSS: %s, Dedicated: %s, LAN: %s, PublicConnections: %d"),
		*SubsystemName.ToString(),
		bIsDedicated ? TEXT("true") : TEXT("false"),
		Settings->bIsLANMatch ? TEXT("true") : TEXT("false"),
		MaxPlayer);
	
	//데디케이트 서버는 로컬 플레이어가 없으므로 인덱스 0으로 직접 호출
	if (bIsDedicated)
	{
		bSuccess = SessionInterface->CreateSession(0, NAME_GameSession, *Settings);
	}
	else
	{
		// 기존 코드
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (LocalPlayer && LocalPlayer->GetPreferredUniqueNetId().IsValid())
		{
			bSuccess = SessionInterface->CreateSession(
				*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *Settings);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Session] LocalPlayer 또는 UniqueNetId가 없어 세션을 생성할 수 없습니다."));
		}
	}

	if (bSuccess == false)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("[Session] CreateSession 호출이 즉시 실패했습니다. OSS: %s, Dedicated: %s"),
			*SubsystemName.ToString(),
			bIsDedicated ? TEXT("true") : TEXT("false"));
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
	UE_LOG(LogTemp, Log, TEXT("[Session] CreateSession %s"), bWasSuccessful ? TEXT("성공") : TEXT("실패"));
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
	UE_LOG(LogTemp, Log, TEXT("[Session] DestroySession %s"), bWasSuccessful ? TEXT("성공") : TEXT("실패"));
	OnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void USMSessionSubsystem::OnSessionUserInviteAcceptedInternal(bool bWasSuccessful, int32 LocalUserNum,
                                                              FUniqueNetIdPtr UserId,
                                                              const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogTemp, Log, TEXT("[SessionSubsystem] Steam 초대 수락 - bWasSuccessful: %s"),
	   bWasSuccessful ? TEXT("true") : TEXT("false"));
	
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
