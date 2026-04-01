#include "Enemy/MonsterBase.h"

AMonsterBase::AMonsterBase()
{
 	PrimaryActorTick.bCanEverTick = true;

}

void AMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMonsterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

