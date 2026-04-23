// ASMLobbyDummy.h

#pragma once

#include "CoreMinimal.h"
#include "SMMonsterBase.h"
#include "SMLobbyDummy.generated.h"

class USMEnemyHPBarComponent;

UCLASS()
class SAGOMAGIC_API ASMLobbyDummy : public ASMMonsterBase
{
	GENERATED_BODY()

public:
	
	ASMLobbyDummy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<USMEnemyHPBarComponent> EnemyHPBarComponent;
};
