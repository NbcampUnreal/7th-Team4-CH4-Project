#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystemInterface.h"
//#include "../GAS/AttributeSets/SMMonsterAttributeSet.h"

#include "SMMonsterBase.generated.h"

class USMMonsterDataAsset;
class USMMonsterAttributeSet;
class ASMBaseItemDropActor;
class UGameplayEffect;
class UAnimMontage;
enum class EMonsterType : uint8;

UCLASS()
class SAGOMAGIC_API ASMMonsterBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
	ASMMonsterBase();

    /**  IAbilitySystemInterface 구현(외부에서 ASC를 찾을 때 사용) **/
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void ResetMonster();
    
    void ApplyVisuals(USMMonsterDataAsset* DataAsset);

    /** 서버 전용: DataAsset에서 AI·GAS 관련 데이터를 로드하여 적용 */
    void ApplyLogicData(USMMonsterDataAsset* DataAsset);

    UFUNCTION()
    void OnRep_MonsterAssetId();

    /** Wave 종료 시 호출 - BaseCamp에 데미지를 주고 자기 자신 제거 (서버 전용) */
    void SelfKill();

protected:
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;

    /** 실제로 부여된 어빌리티를 관리하기 위한 함수 **/
    void GiveDefaultAbilities();

    /** HP가 0이 됐을 때 AttributeSet의 델리게이트로 호출 (서버 전용) **/
    void HandleDeath(AController* KillerController);

    /** 공용 드롭 테이블에서 가중치 기반 랜덤 1개 아이템을 월드에 스폰 (서버 전용) */
    void SpawnDropItem();

public:
    /** 테스트용 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh|Test")
    TSoftObjectPtr<USkeletalMesh> TestMesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drop")
    TSubclassOf<ASMBaseItemDropActor> DropActorClass;
    
    /** 몬스터가 기본적으로 가질 어빌리티 목록 **/
    UPROPERTY(EditAnywhere, Category = "GAS")
    TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;

    /** 몬스터 ASC 컴포넌트* */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> MonsterAbilitySystemComponent;

    /** 몬스터.능력치 세트(HP, MaxHP등등)**/
    UPROPERTY()
    TObjectPtr<USMMonsterAttributeSet> MonsterAttributeSet;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MonsterType")
    EMonsterType MonsterType;

    UPROPERTY(ReplicatedUsing = OnRep_MonsterAssetId)
    FPrimaryAssetId MonsterAssetId;

    /** 자폭 시 1마리당 BaseCamp에 적용할 데미지 양 */
    UPROPERTY(EditDefaultsOnly, Category = "SelfKill")
    float SelfKillDamage = 5.0f;

    /** 아이템 드롭 확률 0.01이 1% */
    UPROPERTY(EditDefaultsOnly, Category = "Drop")
    float DropChance = 0.2f;

    // ── 사운드 ID (DT_Sound DataTable의 SoundName 컬럼과 일치) ──
    /** 공격 시작 사운드 ID */
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    FName AttackSoundID;

    /** 피격 사운드 ID */
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    FName HitSoundID;

    /** 사망 사운드 ID */
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    FName DeathSoundID;
    
    /** DamageEffect를 저장해둘 멤버(런타임에 DataAsset에서 가져옴) */
    UPROPERTY()
    TSubclassOf<UGameplayEffect> CachedDamageEffect;

    /** AttackMontage도 캐싱(어빌리티에서 꺼내 쓸 수 있도록) */
    UPROPERTY()
    TObjectPtr<UAnimMontage> CachedAttackMontage;

    //몬스터 사망 태그 추가 로직
    
    UPROPERTY(ReplicatedUsing=OnRep_IsDead)
    bool bIsDead = false;
    
    UFUNCTION()
    void OnRep_IsDead();

    //클라이언트에서만 실행되는 Death로직, 클라 전용 애니메이션 등 추가 가능
    void HandleClientDeath();

    /** AttributeSet의 OnRep_Health에서 호출 — 클라이언트 피격 사운드 */
    void PlayHitSound();

    /** 서버→모든 클라이언트 공격 사운드 재생 (ServerOnly GA에서 호출) */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayAttackSound();
    
};
