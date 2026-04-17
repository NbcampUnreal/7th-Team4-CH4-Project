// SMSkillTurret.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMSkillTurret.generated.h"

UCLASS()
class SAGOMAGIC_API ASMSkillTurret : public AActor
{
	GENERATED_BODY()

public:
	ASMSkillTurret();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
