// SMPlayerAnimInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SMPlayerAnimInstance.generated.h"

class ASMPlayerCharacter;
/**
 * 캐릭터 ABP의 부모 C++클래스
 */
UCLASS()
class SAGOMAGIC_API USMPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
	TObjectPtr<ASMPlayerCharacter> SMCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDead = false;
};
