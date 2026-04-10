#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameplayTagContainer.h"    
#include "SMMonsterAnimInstance.generated.h"

class ASMMonsterBase;
class UAbilitySystemComponent;

UCLASS()
class SAGOMAGIC_API USMMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
    virtual void NativeInitializeAnimation() override;

    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    /** BT/AI에서 이동 중인지 여부 **/
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float Speed = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsDead = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsAttacking = false;

private:
    UPROPERTY()
    TObjectPtr<ASMMonsterBase> OwnerMonster;

    /**  ASC 캐싱(매 틱 Cast 방지)**/
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> CachedASC;

    // 태그 한 번만 초기화
    FGameplayTag AttackingTag;
};
