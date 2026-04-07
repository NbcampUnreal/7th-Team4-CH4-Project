#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Enemy/SMMonsterBase.h"
#include "Engine/DataAsset.h"
#include "SMMonsterDataAsset.generated.h"

/**
 * 몬스터 한 종류의 에셋 정보를 담는 PrimaryDataAsset
 * 비주얼 에셋은 Soft Reference, GAS 관련은 Hard Reference 유지
 */
UCLASS(BlueprintType)
class SAGOMAGIC_API USMMonsterDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	//AssetManager가 이 에셋을 식별하는 ID
	//Type : "MonsterData"
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FName("MonsterData"),GetFName());
	}
	
	// Monster BP
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Class")
	TSoftClassPtr<ASMMonsterBase> MonsterClass;
	
	// visual
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
	TSoftClassPtr<UAnimInstance> AnimClass;
	
	/** GAS Ability에서 Animation Montage를 호출 하는 경우 추가 필요 */
	//TODO 은서
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|Visual")
	TSoftObjectPtr<UAnimMontage> AttackMontage;
	
	// AI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|AI")
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;
	
	// GAS
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster|GAS")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
