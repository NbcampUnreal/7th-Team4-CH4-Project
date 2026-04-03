#pragma once

#include "CoreMinimal.h"
#include "SMBaseState.h"
#include "SMBuildState.generated.h"

/**
 * 정비 단계 - 카운트다운 타이머가 끝나면 CombatState로 전환
 */
UCLASS()
class SAGOMAGIC_API USMBuildState : public USMBaseState
{
    GENERATED_BODY()
public:
    /** GameState 페이즈 변경 + 타이머 초기화 */
    virtual void Enter() override;

    /** 경과 시간 누적 -> Duration 초과 시 Combat으로 전환 */
    virtual void Tick(float DeltaTime) override;

    /** 타이머 변수 초기화 */
    virtual void Exit() override;

    virtual EGameState GetStateType() const override { return EGameState::Build; }
private:
    /** 총 정비 시간 (초) */
    float Duration = 180.f;

    /** 현재 경과 시간 */
    float Elapsed = 0.f;
};
