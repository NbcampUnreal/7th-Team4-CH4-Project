#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterBase.generated.h"

UCLASS()
class SAGOMAGIC_API AMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	AMonsterBase();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
