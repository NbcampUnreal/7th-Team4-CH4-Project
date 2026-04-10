#pragma once

#include "CoreMinimal.h"
#include "SMGameState.h"
#include "UObject/Object.h"
#include "SMStateMachine.generated.h"

class USMBaseState;
class ASMGameMode;
/**
 *  FSM 본체 - GameMode가 소유, 서버에서만 동작
 *  State 전환 로직과  WaveIndex를 중앙 관리
 */
UCLASS()
class SAGOMAGIC_API USMStateMachine : public UObject
{
    GENERATED_BODY()
public:
    /** GameMode 참조 조장 + 전체 State 등록 + 첫 State(Build) 진입 */
    void Initialize(ASMGameMode* InOwner);
    /** 현재 State에 Tick 위임 - GameMode::Tick에서 호출 */
    void Tick(float DeltaTime);

    /**
     * State가 전환을 요청할 때 호출 (외부 진입점)
     * Combat -> Build 전환 시 WaveIndex 자동 증가
     */
    void ChangeState(EGameState NextState);

    /** State들이 GameMode에 접근할 때 사용 */
    ASMGameMode* GetOwner() const {return Owner;}

    /** State들이 현재 웨이브 번호를 알아야 할 때 사용 */
    int32 GetCurrentWaveIndex() const {return CurrentWaveIndex;}
    
    int32 GetMaxWaveCount() const {return MaxWaveCount;}
    void SetPendingVictory(bool bVictory) {bPendingVictory = bVictory;}
    bool GetPendingVictory() const {return bPendingVictory;}
private:
    /** Build/Combat/Result State 인스터스 생성 후 StateCache에 등록 */
    void RegisterStatus();

    /**
     * 실제 전환 수행
     * 현재 State Exit ->
     * 다음 State 탐색 ->
     * GameState 복제 업데이트 ->
     * 다음 State Enter
     */
    void ApplyState(EGameState NextState);

    /** StateMachine 소유자 - State들이 GameMode 기능에 접근하는 경로 */
    UPROPERTY()
    TObjectPtr<ASMGameMode> Owner;

    /** 현재 실행 중인 State */
    UPROPERTY()
    TObjectPtr<USMBaseState> CurrentState;

    /** EGameState -> State 인스턴스 매핑 */
    UPROPERTY()
    TMap<EGameState, TObjectPtr<USMBaseState>> StateCache;

    /** 현재 State 타입 - RequestTransition에서 Combat->Build 감지에 사용 */
    EGameState CurrentStateType = EGameState::None;

    /** 현재 웨이브 번호 - Combat -> Build 전환마다 증가 */
    int32 CurrentWaveIndex = 1;

    /** 총 웨이브 수 */
    int32 MaxWaveCount = 0;
    
    bool bPendingVictory = false;
};
