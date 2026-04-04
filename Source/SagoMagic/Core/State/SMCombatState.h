#pragma once

#include "CoreMinimal.h"
#include "SMBaseState.h"
#include "SMCombatState.generated.h"

/**
 * 전투 단계 - 몬스터 스폰 시작
 * 전체 몬스터 처치 시 WaveIndex에 따라 Build 또는 Result 전환
 */
UCLASS()
class SAGOMAGIC_API USMCombatState : public USMBaseState
{
    GENERATED_BODY()
public:
    /** WaveManager에 현재 WaveIndex로 웨이브 시작 요청 + OnWaveCleared 바인딩 */
    virtual void Enter() override;

    /** 웨이브 진행은 WaveManager가 처리 - 현재 비워둠 */
    virtual void Tick(float DeltaTime) override;

    /** OnWaveCleared 바인딩 해제 */
    virtual void Exit() override;

    /** OnWaveCleared 바인딩 해제 */
    virtual EGameState GetStateType() const override {return EGameState::Combat;}
private:
    /** 웨이브 클리어 시 호출 - 마지막 웨이브면 Result, 아니면 Build */
    void OnWaveCleared();
private:
    /** 총 정비 시간 (초) */
    float Duration = 180.f;

    /** 현재 경과 시간 */
    float Elapsed = 0.f;
    /** 총 웨이브 수 - StateMachine의 MaxWaveCount와 동일하게 유지 */
    static constexpr int32 MaxWaveCount = 3;
};
