#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
#include "../GAS//AttributeSets/SMMonsterAttributeSet.h"
#include "SMMonsterBase.generated.h"

UCLASS()
class SAGOMAGIC_API ASMMonsterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASMMonsterBase();

    // IAbilitySystemInterface 구현(외부에서 ASC를 찾을 때 사용)
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
protected:
    virtual void BeginPlay() override;
    //virtual void PossessedBy(AController* NewController) override;
public:
    /** 몬스터 ASC 컴포넌트* */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> MonsterAbilitySystemComponent;

    /** 몬스터.능력치 세트(HP, MaxHP등등)**/
    UPROPERTY()
    TObjectPtr<USMMonsterAttributeSet> MonsterAttributeSet;
};

