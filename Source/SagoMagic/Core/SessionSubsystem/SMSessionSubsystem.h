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

//위젯 바인딩용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSMJoinSessionComplete, EOnJoinSessionCompleteResult::Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSMDestroySessionComplete, bool, bWasSuccessful);
UCLASS()
class SAGOMAGIC_API USMSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    USMSessionSubsystem();

    //서브 시스템 인터페이스
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    //위젯 호출용
    void CreateSession(int32 MaxPlayer);
    void JoinSession(const FOnlineSessionSearchResult& SessionResult);
    void DestroySession();

    //위젯 바인딩용 델리게이트
    FOnSMCreateSessionComplete OnCreateSessionComplete;
    FOnSMJoinSessionComplete OnJoinSessionComplete;
    FOnSMDestroySessionComplete OnDestroySessionComplete;

private:
    IOnlineSessionPtr SessionInterface;

    //델리게이트 핸들
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle JoinSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle InviteAcceptedDelegateHandle;

    //내부 콜백용 함수
    /** 세션 생성 성공 콜백 함수 */
    void OnCreateSessionCompleteInternal(FName SessionName, bool bWasSuccessful);

    /** 세션 조인 성공 콜백 함수 */
    void OnJoinSessionCompleteInternal(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

    /** 세션 파괴 성공 콜백 함수 */
    void OnDestroySessionCompleteInternal(FName SessionName, bool bWasSuccessful);

    /** 초대 수락 성공 콜백 함수 */
    void OnSessionUserInviteAcceptedInternal(
        bool bWasSuccessful,
        int32 LocalUserNum,
        FUniqueNetIdPtr UserId,
        const FOnlineSessionSearchResult& InviteResult);

    /** 세션 인터페이스 벨리데이션 함수 */
    bool IsValidSessionInterface();
};
