#pragma once

#include "CoreMinimal.h"
#include "SMMonsterSpawner.h"
#include "Subsystems/WorldSubsystem.h"
#include "SMWaveManagerSubsystem.generated.h"


/**
 *스폰 항목 하나를 실시간으로 처리하는 내부 구조체
 *SpawnCount만큼 SpawnInterval 간격으로 몬스터 스폰
 */
USTRUCT()
struct FActiveSpawnTask
{
    GENERATED_BODY()

    /** 소활할 몬스터 클래스 (TSoftClassPtr 로드 완료 후 저장) */
    TSubclassOf<APawn> MonsterClass;

    /**남은 스폰 수*/
    int32 RemainingCount = 0;

    /** 소환 간격 (초)*/
    float Interval = 0.5f;

    /** 현재 누적 시간 */
    float AccumulatedTime = 0.f;

};

/**
 * Wave 스폰 전담 Subsystem - 서버에서만 실제 로직 동작
 * CombatState가 StartWave()를 호출하고,
 * 몬스터 전멸 시 GM->OnWaveCleared.ExecuteIfBound() 호출
 */
UCLASS()
class SAGOMAGIC_API USMWaveManagerSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

    /**
     *CombatState::Enter()에서 호출
     *WaveIndex로 DataTable Row 조회 후 스폰 시작
     */
    void StartWave(int32 WaveIndex);

    /**
     *몬스터 사망 시 ASMMonsterBase::Die()에서 호출
     *전멸 감지 후 OnWaveCleared 발동
     */
    void OnMonsterDied();

    /** GameMode BeginPlay()에서 DataTable 주입 */
    void SetWaveDataTable(UDataTable* InDataTable);
    UDataTable* GetWaveDataTable();

private:
    /**레벨에 배치된 모든 Spawner 수집 */
    void CollectSpawners();

    /** Spawners 배열에서 무작위 하나 반환 */
    ASMMonsterSpawner* GetRandomSpawner() const;

    /** 몬스터 1마리 스폰 + 생존 카운트 증가*/
    void SpawnOne(TSubclassOf<APawn> MonsterClass);

    /** 모든 몬스터 전멸 + 스폰 완료 시 WaveCleared 호출 */
    void CheckWaveCleared();
private:

    UPROPERTY()
    TObjectPtr<UDataTable> WaveDataTable;
    /**레벨 내 모든 스포너 */
    UPROPERTY()
    TArray<TObjectPtr<ASMMonsterSpawner>> Spawners;

    /** 현재 처리 중인 스폰 작업 목록 */
    TArray<FActiveSpawnTask> ActiveSpawnTasks;

    /** 현재 살아있는 몬스터 수 */
    int32 AliveMonsterCount =0;

    /** 아직 소환중인 작업이 남았는지 */
    bool bSpawningInProgress = false;

    /** Tick 활성화 제어 */
    bool bIsWaveActive = false;

    /**현재 Wave Index */
    int32 CurrentWaveIndex = 0;

public:

};
