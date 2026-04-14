#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Building/SMBaseBuilding.h"
#include "Building/SMBuildPlaceTargetData.h"
#include "Building/SMGridManager.h"
#include "GA_BuildPlace.generated.h"

/**
 * 건물 배치
 */
UCLASS()
class SAGOMAGIC_API UGA_BuildPlace : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_BuildPlace();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	
private:
	/** 모든 셀이 비어있는지 서버 검사 */
	bool ServerValidateCells(ASMGridManager* GridManager,
		const TArray<FSMCellPlaceInfo>& CellInfos);
	
	/** 비용 GE 적용 */
	bool ApplyBuildCost(
		const FGameplayAbilitySpecHandle& Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo& ActivationInfo,
		EGridBuildingType BuildingType);
	
	/** 스폰 + GridManager 등록 */
	bool SpawnAndRegister(ASMGridManager* GridManager,
		const TArray<FSMCellPlaceInfo>& CellInfos,
		TSubclassOf<AActor> BuildingClass,
		EGridBuildingType BuildingType,
		int32 OwnerId);
	
	void Rollback(ASMGridManager* GridManager,
		TArray<ASMBaseBuilding*>& SpawnedActors);
	ASMGridManager* GetGridManager();
public:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Cost")
	TMap<EGridBuildingType, TSubclassOf<UGameplayEffect>> BuildCostEffects;
private:
	UPROPERTY()
	TObjectPtr<ASMGridManager> CachedGridManager = nullptr;
};

