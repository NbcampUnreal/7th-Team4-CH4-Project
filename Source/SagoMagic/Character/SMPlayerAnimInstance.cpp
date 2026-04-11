// SMPlayerAnimInstance.cpp


#include "Character/SMPlayerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "SMPlayerCharacter.h"

void USMPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	SMCharacter = Cast<ASMPlayerCharacter>(TryGetPawnOwner());
}

void USMPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!SMCharacter) return;
	
	FVector Velocity = SMCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();
	
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, SMCharacter->GetActorRotation());
	
	bIsDead = SMCharacter->IsDead();
}
