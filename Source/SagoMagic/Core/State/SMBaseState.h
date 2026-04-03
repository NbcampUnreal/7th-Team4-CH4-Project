#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/SMGameState.h"
#include "SMBaseState.generated.h"

class ASMGameMode;
class USMStateMachine;
/**
 * 모든 State의 공통 기반 클래스
 * 자식 클래스(Build/Combat/Result)가 Enter/Tick/Exit을 오버라이드하여 로직 구현
 */
UCLASS()
class SAGOMAGIC_API USMBaseState : public UObject
{
    GENERATED_BODY()
public:
    /** StateMachine 참조 주입 - RegisterStatus에서 생성 직후 호출 */
    virtual void Initialize(USMStateMachine* InStateMachine);

    /** State가 활성화될 때 1회 호출 - 초기화, 타이머 시작 등 */
    virtual void Enter();

    /** 활성화된 동안 매 플레임 호출 - 타이머 감산, 조건 감시 등 */
    virtual void Tick(float DeltaTime);

    /** 다음 State로 넘어가기 직전 1회 호출 - 정리, 바인딩 해제 등 */
    virtual void Exit();

    /** 자신의 State 타입 반환 - StateCache 등록 키로 사용 */
    UFUNCTION(BlueprintPure, Category = "State")
    virtual EGameState GetStateType() const PURE_VIRTUAL(USMBaseState::GetStateType, return EGameState::None;);

protected:
    /** StateMachine에 전환 요청 - 자식 State에서 조건 충족 시 호출 */
    void ChangeState(EGameState NewState);

    /** StateMachine -> Owner(GameMode) 경로로 GameMode 참조 반환 */
    ASMGameMode* GetGameMode() const;

    /** GameMode -> GetGameState 경로로 GameState 참조 반환 */
    ASMGameState* GetGameState() const;
protected:
    /** 소유 StateMachine - GC 추적용, 저장 불필요하므로 Transient */
    UPROPERTY(Transient)
    TObjectPtr<USMStateMachine> StateMachine;

};
