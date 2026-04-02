#pragma once
#include "Core/SMGameMode.h"
/**
 * GamePhase의 기본이 되는 Class
 */
class IGamePhaseCommand
{
public:
    virtual ~IGamePhaseCommand() = default;
    /** 페이즈 시작 시 호출 */
    virtual void Enter(ASMGameMode* GM) = 0;
    /** 페이즈 매 프레임 호출(서버 Tick) */
    virtual void Tick(float DeltaTime) = 0;
    /** 페이즈 종료 시 호출 */
    virtual void Exit(ASMGameMode* GM) = 0;
    /** 완료 신호 - Queue가 여기에 다음 꺼내기를 바인딩 */
    FSimpleDelegate OnCompleted;
};
