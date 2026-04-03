#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
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

    UPROPERTY(EditAnywhere, Category = "AI")
    class UBehaviorTree* BTAsset;

    /** 에디터에서 할당할 블랙보드 에셋 **/
    UPROPERTY(EditAnywhere, Category = "AI")
    class UBlackboardData* BBAsset;

    /** 실제로 런타임에 데이터를 담을 컴포넌트 **/
    UPROPERTY()
    class UBlackboardComponent* BlackboardComp;
};
