#pragma once

#include "CoreMinimal.h"
#include "SMMonsterSpawner.h"
#include "Enemy/SMMonsterBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "SMWaveManagerSubsystem.generated.h"

class USMMonsterDataAsset;

DECLARE_DELEGATE(FOnReadyForCombat);
/**
 * Wave 스폰 전담 Subsystem - 서버에서만 실제 로직 동작
 * CombatState가 StartWave()를 호출하고,
 * 몬스터 전멸 시 GM->OnWaveCleared.ExecuteIfBound() 호출
 */
UCLASS()
class SAGOMAGIC_API USMWaveManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Deinitialize() override;
    
    static USMWaveManagerSubsystem* Get(const UObject* WorldContextObject);
    
    //BuildState::Enter()에서 호출
    void PreSpawnForWave(int32 WaveIndex);
    
    //CombatState::Enter()에서 호출
    void StartWave(int32 WaveIndex);

    //MonsterBase::HandleDeath()에서 호출
    void OnMonsterDied(ASMMonsterBase* Monster);
    
    //preSpawn 완료 시 발동 -> BuildState가 Combat으로 전환
    FOnReadyForCombat OnReadyForCombat;
    
    /** 클라이언트 로드 완료 시 GameState에서 호출 */
    void OnClientLoadComplete();
    
    /** 서버 + 모든 클라이언트 로드 완료 혹인 */
    void CheckAllReady();
    
private:
    /** SetTimer로 PreLoading을 담당하는 함수 */
    void TickPreSpawning();
    /** StartWave에서 SpawnInterval에 맞게 Random위치에 스폰해주는 함수 */
    void TickActivation();
    /**레벨에 배치된 모든 Spawner 수집 */
    void CollectSpawners();
    
    /**SetTimer로 Destroy를 담당하는 함수 */
    void TickDestroy();
    /**Destroy할 monster를 등록하는 함수 */
    void ScheduleDestroy(ASMMonsterBase* Monster);
    
    /** Spawners 배열에서 무작위 하나 반환 */
    ASMMonsterSpawner* GetRandomSpawner() const;

    /** 모든 몬스터 전멸 + 스폰 완료 시 WaveCleared 호출 */
    void CheckWaveCleared();
private:
    /**레벨 내 모든 스포너 */
    UPROPERTY()
    TArray<TObjectPtr<ASMMonsterSpawner>> Spawners;

    /** 스폰 큐 [(MonsterType, SpawnCout)] */
    TArray<TPair<EMonsterType, int32>> SpawnQueue;
    
    /** 타입별 AssetId 매핑 */
    TMap<EMonsterType, FPrimaryAssetId> MonsterAssetIDs;

    /** 미리 스폰된 Hidden Actor 풀 */
    UPROPERTY()
    TArray<TObjectPtr<ASMMonsterBase>> PreSpawnedActors;
    
    /** 활성화된 살아있는 몬스터 */
    UPROPERTY()
    TArray<TObjectPtr<ASMMonsterBase>> AliveMonsters;
    
    /** 분산 Destroy 대기 목록 */
    UPROPERTY()
    TArray<TObjectPtr<ASMMonsterBase>> PendingDestroyActors;
    
    /** PreLoading을 담당하는 핸들러 */
    FTimerHandle PreSpawnTimerHandle;
    /** Spawn을 담당하는 핸들러 */
    FTimerHandle ActivateTimerHandle;
    /** Destroy 타이머 핸들 */
    FTimerHandle DestroyTimerHandle;
    
    bool bServerReady = false;
    int32 ReadyClientCount = 0;
    int32 TotalExpectedClients = 0;
    
};
