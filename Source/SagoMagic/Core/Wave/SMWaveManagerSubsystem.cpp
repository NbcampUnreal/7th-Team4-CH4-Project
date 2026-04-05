#include "SMWaveManagerSubsystem.h"

#include "EngineUtils.h"
#include "Core/SMGameMode.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "Data/SMMonsterData.h"
#include "GameFramework/Character.h"
#include "Data/SMWaveData.h"

bool USMWaveManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    if (World->GetNetMode() == NM_Client) return false;//클라이언트 차단
    return World->GetMapName().Contains(TEXT("L_Play"));//L_Play에서만 생성

}

void USMWaveManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USMWaveManagerSubsystem::Deinitialize()
{
    ActiveSpawnTasks.Empty();
    Spawners.Empty();
    Super::Deinitialize();
}

void USMWaveManagerSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GetWorld()->GetNetMode() == NM_Client) return;

    bool bAnyTaskRemaining = false;
    for (FActiveSpawnTask & Task : ActiveSpawnTasks)
    {
        if (Task.RemainingCount <= 0) continue;

        bAnyTaskRemaining = true;
        Task.AccumulatedTime += DeltaTime;

        if (Task.AccumulatedTime >= Task.Interval)
        {
            Task.AccumulatedTime = 0.f;
            Task.RemainingCount--;
            SpawnOne(Task.MonsterClass);
        }
    }

    //몯든 Task 소진 시 스폰 완료
    if (!bAnyTaskRemaining && bSpawningInProgress)
    {
        bSpawningInProgress = false;
        UE_LOG(LogTemp, Log, TEXT("[WaveManager] 전체 스폰 완료 - 생존 : %d"),AliveMonsterCount);
        CheckWaveCleared();//스폰 완료 시점에 이미 전멸했을 경우 방지
    }
}

TStatId USMWaveManagerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USMWaveManagerSubsystem, STATGROUP_Tickables);
}

bool USMWaveManagerSubsystem::IsTickable() const
{
    return bIsWaveActive;
}

void USMWaveManagerSubsystem::StartWave(int32 WaveIndex)
{
    //서버에서만 동작
    if (GetWorld()->GetNetMode() == NM_Client) return;

    USMSyncDataManager* DM = USMSyncDataManager::Get(this);
    if (!DM)
    {
        UE_LOG(LogTemp, Error, TEXT("[WaveManager] DataManager 없음"));
        return;
    }
    FSMWaveData WaveData = DM->GetWaveData(WaveIndex);
    
    ActiveSpawnTasks.Empty();
    AliveMonsterCount = 0;
    bIsWaveActive = true;
    bSpawningInProgress = true;

    CollectSpawners();

    if (Spawners.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[WaveManager] 레벨에 Spawner 없음"));
        bIsWaveActive = false;
        bSpawningInProgress = false;
        return;
    }

    for (const FWaveSpawnData& Entry : WaveData.SpawnList)
    {
        FSMMonsterData MonsterData = DM->GetMonsterData(Entry.MonsterType);
        TSubclassOf<ASMMonsterBase> MonsterClass = MonsterData.MonsterClass.LoadSynchronous();
        if (!MonsterClass)
        {
            UE_LOG(LogTemp, Warning, TEXT("[WaveManager] MonsterClass 로드 실패"));
            continue;
        }

        FActiveSpawnTask Task;
        Task.MonsterClass = MonsterClass;
        Task.RemainingCount = Entry.SpawnCount;
        Task.Interval = Entry.SpawnInterval;
        Task.AccumulatedTime = Entry.SpawnInterval;

        ActiveSpawnTasks.Add(Task);
    }

    UE_LOG(LogTemp, Log, TEXT("[WaveManager] Wave %d 시작 - SpawnTask %d개"),
        WaveIndex, ActiveSpawnTasks.Num());

}

void USMWaveManagerSubsystem::OnMonsterDied()
{
    if (GetWorld()->GetNetMode() == NM_Client) return;

    AliveMonsterCount = FMath::Max(0, AliveMonsterCount -1);
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] 몬스터 사망 - 생존 : %d"), AliveMonsterCount);

    CheckWaveCleared();
}

void USMWaveManagerSubsystem::CollectSpawners()
{
    Spawners.Empty();
    for (TActorIterator<ASMMonsterSpawner> It(GetWorld()); It; ++It)
    {
        Spawners.Add(*It);
    }
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] Spawner %d개 수집"), Spawners.Num());
}

ASMMonsterSpawner* USMWaveManagerSubsystem::GetRandomSpawner() const
{
    if (Spawners.IsEmpty()) return nullptr;

    int32 RandomIndex = FMath::RandRange(0, Spawners.Num() - 1);
    return Spawners[RandomIndex];
}

void USMWaveManagerSubsystem::SpawnOne(TSubclassOf<ASMMonsterBase> MonsterClass)
{
    ASMMonsterSpawner* Spawner = GetRandomSpawner();
    if (!Spawner) return;

    ASMMonsterBase* Monster = Spawner->SpawnMonster(MonsterClass);
    if (Monster)
    {
        AliveMonsterCount++;
        //TODO 은서 : 몬스터 Die 함수에서 OnMonsterDied() 호출 연결 필요
    }
}

void USMWaveManagerSubsystem::CheckWaveCleared()
{
    //스폰이 아직 진행 중이면 아직 아님
    if (bSpawningInProgress) return;

    //살아있는 몬스터가 남아있으면 아직 아님
    if (AliveMonsterCount > 0) return;

    bIsWaveActive = false;
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] 웨이브 클리어!"));

    if(ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>())
        GM->OnWaveCleared.ExecuteIfBound();
}
