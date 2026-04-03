#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "SMMonsterAIController.generated.h"

UCLASS()
class SAGOMAGIC_API ASMMonsterAIController : public AAIController
{
	GENERATED_BODY()
public:
    ASMMonsterAIController();
protected:
    virtual void OnPossess(APawn* InPawn) override;

public:
    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    // AI Perception 컴포넌트 참조 (보통 생성자에서 생성)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* PerceptionComp;

    UPROPERTY(EditAnywhere, Category = "AI")
    class UBehaviorTree* BTAsset;

    /** 에디터에서 할당할 블랙보드 에셋 **/
    UPROPERTY(EditAnywhere, Category = "AI")
    class UBlackboardData* BBAsset;

    /** 실제로 런타임에 데이터를 담을 컴포넌트 **/
    UPROPERTY()
    class UBlackboardComponent* BlackboardComp;
};
