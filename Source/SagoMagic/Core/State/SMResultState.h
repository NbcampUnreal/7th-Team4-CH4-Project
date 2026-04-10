#pragma once

#include "CoreMinimal.h"
#include "SMBaseState.h"
#include "SMResultState.generated.h"

/**
 *  결과 단계 - 승리 또는 패배 처리
 *  보스 처치 시 승리 / 베이스캠프 파괴 시 패배로 진입
 */
UCLASS()
class SAGOMAGIC_API USMResultState : public USMBaseState
{
    GENERATED_BODY()
public:
    /** 승패 결과 판단 + 결과 UI 표시 요청 */
    virtual void Enter() override;

    /** 결과 화면 대기 - 현재 비워둠 */
    virtual void Tick(float DeltaTime) override;

    /** 상태 초기화 */
    virtual void Exit() override;

    virtual EGameState GetStateType() const override {return EGameState::Result;}
};
