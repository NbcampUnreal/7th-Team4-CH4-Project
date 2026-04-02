// SMSessionSubsystem.h

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SMSessionSubsystem.generated.h"

/**
 * 세션 생성및 연결을 도와주는 서브시스템
 * 스팀과 연동되어 친구 초대 및 수락 가능
 */

/** 위젯 바인딩용 델리게이트*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSMJoinSessionComplete, EOnJoinSessionCompleteResult::Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMDestroySessionComplete, bool, bWasSuccessful);
UCLASS()
class SAGOMAGIC_API USMSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    USMSessionSubsystem();

    /** 서브시스템 인터페이스 */
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** 위젯 호출용 */
    void CreateSession(int32 MaxPlayer);
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);
    void DestroySession();

    /** 위젯 바인딩 델리게이트 */
    FOnSMCreateSessionComplete OnCreateComplete;
    FOnSMJoinSessionComplete OnJoinSessionComplete;
    FOnSMDestroySessionComplete OnDestroyComplete;

private:
    IOnlineSessionPtr SessionInterface;

    /** 델리게이트 핸들 */
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle InviteAcceptedDelegateHandle;

    /** 내부 콜백 (엔진 호출) */
    void OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful);
    void OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
    void OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful);
    void OnSessionUserInviteAcceptedInternal(
        bool bWasSuccessful,
        int32 LocalUserNum,
        FUniqueNetIdPtr UserId,
        const FOnlineSessionSearchResult& InviteResult);
    bool IsValidSessionInterface();
};
