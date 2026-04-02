#pragma once
#include "IGamePhaseCommand.h"
/**
 * 전투 Phase를 담당하는 class
 */
class FCombatCommand: public IGamePhaseCommand
{
public:
    virtual ~FCombatCommand();
    /**페이즈 시작 시 호출*/
    virtual void Enter(ASMGameMode* GM);
    /**페이즈 매 프레임 호출(서버 Tick)*/
    virtual void Tick(float DeltaTime);
    /**페이즈 종료 시 호출*/
    virtual void Exit(ASMGameMode* GM);
};
