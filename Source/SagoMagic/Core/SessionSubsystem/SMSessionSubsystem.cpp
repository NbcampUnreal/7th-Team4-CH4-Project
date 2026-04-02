// Fill out your copyright notice in the Description page of Project Settings.


#include "SMSessionSubsystem.h"

FOnSMCreateSessionComplete::FOnSMCreateSessionComplete()
{
}

FOnSMCreateSessionComplete::FOnSMCreateSessionComplete(const TMulticastScriptDelegate<>& InMulticastScriptDelegate)
{
}

void FOnSMCreateSessionComplete::FOnSMCreateSessionComplete_DelegateWrapper(const FMulticastScriptDelegate&,
    bool bWasSuccessful)
{
}

FOnSMDestroySessionComplete::FOnSMDestroySessionComplete()
{
}

FOnSMDestroySessionComplete::FOnSMDestroySessionComplete(const TMulticastScriptDelegate<>& InMulticastScriptDelegate)
{
}

void FOnSMDestroySessionComplete::FOnSMDestroySessionComplete_DelegateWrapper(const FMulticastScriptDelegate&,
    bool bWasSuccessful)
{
}

USMSessionSubsystem::USMSessionSubsystem()
{
}

void USMSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USMSessionSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void USMSessionSubsystem::CreateSession(int32 MaxPlayer)
{
}

void USMSessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
}

void USMSessionSubsystem::DestroySession()
{
}

void USMSessionSubsystem::OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
}

void USMSessionSubsystem::OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
}

void USMSessionSubsystem::OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful)
{
}

void USMSessionSubsystem::OnSessionUserInviteAcceptedInternal(bool bWasSuccessful, int32 LocalUserNum,
    FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
}

bool USMSessionSubsystem::IsValidSessionInterface()
{
    return true;
}
