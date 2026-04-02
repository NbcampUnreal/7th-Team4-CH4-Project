#pragma once
#include "IGamePhaseCommand.h"

/**
 * 건축 phase를 담당하는 클래스
 */
class FBuildCommand : public IGamePhaseCommand
{
public:
    virtual ~FBuildCommand();
    /**페이즈 시작 시 호출*/
    virtual void Enter(ASMGameMode* GM);
    /**페이즈 매 프레임 호출(서버 Tick)*/
    virtual void Tick(float DeltaTime);
    /**페이즈 종료 시 호출*/
    virtual void Exit(ASMGameMode* GM);
};
