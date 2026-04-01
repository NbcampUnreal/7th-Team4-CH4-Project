#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SMMonsterAIController.generated.h"

UCLASS()
class SAGOMAGIC_API ASMMonsterAIController : public AAIController
{
	GENERATED_BODY()
public:
    ASMMonsterAIController();
protected:
    virtual void OnPossess(APawn* InPawn) override;
};
