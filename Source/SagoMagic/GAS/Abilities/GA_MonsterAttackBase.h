#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MonsterAttackBase.generated.h"

UCLASS()
class SAGOMAGIC_API UGA_MonsterAttackBase : public UGameplayAbility
{
	GENERATED_BODY()
public:
    UGA_MonsterAttackBase();

    /** 어빌리티가 실행될 때 호출되는 핵심 함수**/
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
    /** 사용할 공격 애니메이션 **/
    UPROPERTY(EditAnywhere, Category = "Design")
    UAnimMontage* AttackMontage;

    /** 타격 판정 시점에 발생할 태그 (AnimNotify에서 보낼 태그) **/
    UPROPERTY(EditAnywhere, Category = "Design")
    FGameplayTag HitEventTag;

    /**
     * 타겟에게 적용할 데미지 GameplayEffect 클래스.
     * 에디터(블루프린트 서브클래스)에서 GE_MonsterDamage 같은 에셋을 할당하세요.
     **/
    UPROPERTY(EditAnywhere, Category = "Design")
    TSubclassOf<class UGameplayEffect> DamageEffectClass;

    /**
     * 공격 판정 반경 (SphereTrace에 사용, cm 단위).
     * 에디터에서 몬스터 종류에 맞게 조정하세요.
     **/
    UPROPERTY(EditAnywhere, Category = "Design")
    float AttackRadius = 80.0f;

    /**
     * 공격 판정 거리 - 몬스터 앞쪽으로 얼마나 뻗을지 (cm 단위).
     **/
    UPROPERTY(EditAnywhere, Category = "Design")
    float AttackRange = 150.0f;

    /** 타격 이벤트 수신 시 실행될 콜백 **/
    UFUNCTION()
    void OnHitEventReceived(FGameplayEventData Payload);

private:
    /**
     * 몬스터 앞 방향으로 SphereTrace를 쏴서 타격 대상을 찾습니다.
     * @param OutHitResult - 맞은 대상 정보
     * @return 타격 성공 여부
     **/
    bool PerformHitCheck(FHitResult& OutHitResult) const;

    /**
     * 몬스터 AttributeSet에서 현재 AttackPower 값을 읽어옵니다.
     * AttributeSet이 없으면 기본값 10을 반환합니다.
     **/
    float GetMonsterAttackPower() const;
};
