#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
//#include "../GAS/AttributeSets/SMMonsterAttributeSet.h"

#include "SMMonsterBase.generated.h"

class USMMonsterAttributeSet;
enum class EMonsterType : uint8;

UCLASS()
class SAGOMAGIC_API ASMMonsterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
	ASMMonsterBase();

    // IAbilitySystemInterface 구현(외부에서 ASC를 찾을 때 사용)
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    void ResetMonster();

    //UFUNCTION(NetMulticast, Reliable)
    //void MulticastHandleDeath();
protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

    /** 실제로 부여된 어빌리티를 관리하기 위한 함수 **/
    void GiveDefaultAbilities();

    /** HP가 0이 됐을 때 AttributeSet의 델리게이트로 호출 (서버 전용) **/
    void HandleDeath(AController* KillerController);

    UPROPERTY(EditAnywhere, Category = "Reward")
    float GoldReward = 10.0f;
public:
    /** 몬스터가 기본적으로 가질 어빌리티 목록 **/
    UPROPERTY(EditAnywhere, Category = "GAS")
    TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

    /** 몬스터 ASC 컴포넌트* */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> MonsterAbilitySystemComponent;

    /** 몬스터.능력치 세트(HP, MaxHP등등)**/
    UPROPERTY()
    TObjectPtr<USMMonsterAttributeSet> MonsterAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    EMonsterType MonsterType;
};

