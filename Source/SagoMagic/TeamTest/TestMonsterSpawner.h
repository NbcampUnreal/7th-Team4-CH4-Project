#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestMonsterSpawner.generated.h"

class ASMMonsterBase;
class USMMonsterDataAsset;

UCLASS()
class SAGOMAGIC_API ATestMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATestMonsterSpawner();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnMonster();

	/** 서버 + 모든 클라이언트에 비주얼 직접 적용 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyVisuals(ASMMonsterBase* Monster);

	/** 스폰할 몬스터 DataAsset (MonsterClass / Mesh / AI / Ability 모두 포함) */
	UPROPERTY(EditAnywhere, Category = "Test|Spawn")
	TObjectPtr<USMMonsterDataAsset> MonsterDataAsset;

	/** 스폰 간격 (초) */
	UPROPERTY(EditAnywhere, Category = "Test|Spawn")
	float SpawnInterval = 1.0f;

	/** 최대 스폰 수 (0 = 무제한) */
	UPROPERTY(EditAnywhere, Category = "Test|Spawn")
	int32 MaxSpawnCount = 0;

	FTimerHandle SpawnTimerHandle;
	int32 CurrentSpawnCount = 0;
};
