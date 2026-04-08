#include "SMWaveManagerSubsystem.h"

#include "EngineUtils.h"
#include "Core/SMGameMode.h"
#include "Core/SMGameState.h"
#include "Core/DataManager/SMAsyncDataManager.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "Data/SMMonsterDataAsset.h"
#include "Data/SMMonsterData.h"
#include "Data/SMWaveData.h"
#include "Engine/AssetManager.h"

bool USMWaveManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    UWorld* World = Cast<UWorld>(Outer);
    if (!World) return false;
    if (World->GetNetMode() == NM_Client) return false;//클라이언트 차단
    return World->GetMapName().Contains(TEXT("L_Play"));//L_Play에서만 생성

}

void USMWaveManagerSubsystem::Deinitialize()
{
    GetWorld()->GetTimerManager().ClearTimer(PreSpawnTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(ActivateTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
    
    SpawnQueue.Empty();
    PreSpawnedActors.Empty();
    AliveMonsters.Empty();
    Spawners.Empty();
    PendingDestroyActors.Empty();
    Super::Deinitialize();
}

USMWaveManagerSubsystem* USMWaveManagerSubsystem::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject) return nullptr;
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) return nullptr;
    return World->GetSubsystem<USMWaveManagerSubsystem>();
}

void USMWaveManagerSubsystem::PreSpawnForWave(int32 WaveIndex)
{
    if (GetWorld()->GetNetMode() == NM_Client) return;
    
    //이전 웨이브 남은 Actor Destroy
    for (ASMMonsterBase* Actor : PreSpawnedActors)
    {
        if (IsValid(Actor))
            Actor->Destroy();
    }
    for (ASMMonsterBase* Actor : AliveMonsters)
    {
        if (IsValid(Actor))
            Actor->Destroy();
    }
    SpawnQueue.Empty();
    MonsterAssetIDs.Empty();
    PreSpawnedActors.Empty();
    AliveMonsters.Empty();
    PendingDestroyActors.Empty();
    
    USMSyncDataManager* DM = USMSyncDataManager::Get(this);
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    if (!DM || !AM) return;
    
    FSMWaveData WaveData = DM->GetWaveData(WaveIndex);
    
    
    //WaveData에서 필요한 DataAsset들 ID뽑아오는 작업
    TArray<FPrimaryAssetId> IDsToLoad;
    for (const FWaveSpawnData& Entry : WaveData.SpawnList)
    {
        SpawnQueue.Add(TPair<EMonsterType, int32>(Entry.MonsterType, Entry.SpawnCount));
        
        FSMMonsterData MonsterData = DM->GetMonsterData(Entry.MonsterType);
        FPrimaryAssetId AssetID = UAssetManager::Get()
            .GetPrimaryAssetIdForPath(MonsterData.MonsterClass.ToSoftObjectPath());
        
        if (AssetID.IsValid())
        {
            MonsterAssetIDs.Add(Entry.MonsterType, AssetID);
            IDsToLoad.AddUnique(AssetID);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[WaveManager] %d 타입 AssetID 없음"),
                (int32)Entry.MonsterType);
        }
    }
    
    if (IDsToLoad.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("[WaveManager] 로드할 Asset 없음 - OnReadyForCombat 즉시 발동"));
        OnReadyForCombat.ExecuteIfBound();
        return;
    }

    bServerReady = false;
    ReadyClientCount = 0;
    TotalExpectedClients = GetWorld()->GetGameState()->PlayerArray.Num();
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] TotalExpectedClients: %d"), TotalExpectedClients);
    if (ASMGameState* GS = GetWorld()->GetGameState<ASMGameState>())
    {
        GS->MulticastPreloadClientAssets(IDsToLoad);
    }
    
    // 서버 로드 완료 콜백
    AM->LoadAssetsByIDWithBundles(IDsToLoad, TArray<FName>{"Server"},
        FOnAssetLoadComplete::CreateLambda([this]()
        {
            UE_LOG(LogTemp, Log, TEXT("[WaveManager] 서버 DataAsset 로드 완료"));
            bServerReady = true;
            CheckAllReady();
        })
    );
}

void USMWaveManagerSubsystem::TickPreSpawning()
{
    //스폰 큐 확인
    if (SpawnQueue.IsEmpty())
    {
        GetWorld()->GetTimerManager().ClearTimer(PreSpawnTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("[WaveManager] PreSpawn 완료 (%d개) - OnReadyForCombat"),
            PreSpawnedActors.Num());
        OnReadyForCombat.ExecuteIfBound();
        return;
    }
    
    TPair<EMonsterType, int32>& Front = SpawnQueue[0];
    EMonsterType Type = Front.Key;
    
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    FPrimaryAssetId* AssetIdPtr = MonsterAssetIDs.Find(Type);
    
    if (!AM || !AssetIdPtr)
    {
        Front.Value--;
        if (Front.Value <= 0) SpawnQueue.RemoveAt(0);
        return;
    }
    
    //DataAsset확인
    USMMonsterDataAsset* DataAsset = Cast<USMMonsterDataAsset>(AM->GetLoadAsset(*AssetIdPtr));
    if (!DataAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WaveManager] DataAsset 캐스트 실패"));
        Front.Value--;
        if (Front.Value <= 0) SpawnQueue.RemoveAt(0);
        return;
    }
    
    //MonsterClass 로드(껍대기만 로드하는 방식이여서 로드가 안무거움)
    TSubclassOf<ASMMonsterBase> MonsterClass = DataAsset->MonsterClass.LoadSynchronous();
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WaveManager] MonsterClass 로드 실패"));
        Front.Value--;
        if (Front.Value <= 0) SpawnQueue.RemoveAt(0);
        return;
    }
    
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    //몬스터 스폰
    ASMMonsterBase* Monster = GetWorld()->SpawnActor<ASMMonsterBase>(
        MonsterClass,
        FVector(0.f,0.f,-10000.f),
        FRotator::ZeroRotator,
        Params
    );
    //몬스터 숨김
    if (Monster)
    {
        Monster->MonsterType = Type;
        Monster->SetActorHiddenInGame(true);
        Monster->SetActorEnableCollision(false);
        PreSpawnedActors.Add(Monster);
    }
    
    Front.Value--;
    if (Front.Value <= 0)
        SpawnQueue.RemoveAt(0);
}

void USMWaveManagerSubsystem::TickActivation()
{
    if (PreSpawnedActors.IsEmpty())
    {
        GetWorld()->GetTimerManager().ClearTimer(ActivateTimerHandle);
        CheckWaveCleared();
        return;
    }
    
    ASMMonsterBase* Monster = PreSpawnedActors[0];
    PreSpawnedActors.RemoveAt(0);
    
    if (!IsValid(Monster)) return;
    
    //DataAsset -> DamageEffect 적용 (서버 전용)
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    FPrimaryAssetId* AssetIdPtr = MonsterAssetIDs.Find(Monster->MonsterType);
    if (AM && AssetIdPtr)
    {
        Monster->MonsterAssetId = *AssetIdPtr;
        USMMonsterDataAsset* DataAsset =
            Cast<USMMonsterDataAsset>(AM->GetLoadAsset(*AssetIdPtr));
        if (DataAsset)
        {
            TSubclassOf<UGameplayEffect> EffectClass = DataAsset->DamageEffect.LoadSynchronous();
            // TODO 은서 : EffectClass를 Monster의 GA에 주입
            // if (EffectClass && Monster->MonsterAbilitySystemComponent)
            // {
            //     for (FGameplayAbilitySpec& Spec : Monster->MonsterAbilitySystemComponent->GetActivatableAbilities())
            //     {
            //         if (UGA_MonsterAttackBase* GA = Cast<UGA_MonsterAttackBase>(Spec.Ability))
            //         {
            //             GA->DamageEffectClass = EffectClass;
            //         }
            //     }
            // }
        }
    }
    
    //랜덤 Spawner 위치로 이동
    if (ASMMonsterSpawner* Spawner = GetRandomSpawner())
    {
        Monster->SetActorLocationAndRotation(
            Spawner->GetActorLocation(),
            Spawner->GetActorRotation()
        );
    }
    //활성화
    Monster->SetActorHiddenInGame(false);
    Monster->SetActorEnableCollision(true);
    Monster->SetActorTickEnabled(true);
    
    //AI 재시작
    Monster->SpawnDefaultController();
    
    AliveMonsters.Add(Monster);
    
    UE_LOG(LogTemp, Log, TEXT("[WaveManger] 몬스터 활성화 - 생존 : %d / 대기 : %d"),
        AliveMonsters.Num(),PreSpawnedActors.Num());
}

void USMWaveManagerSubsystem::StartWave(int32 WaveIndex)
{
    //서버에서만 동작
    if (GetWorld()->GetNetMode() == NM_Client) return;

   CollectSpawners();
    if (Spawners.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[WaveManager] 레벨에 Spawner 없음"));
        return;
    }
    
    float SpawnInterval = 1.0f;
    USMSyncDataManager* DM = USMSyncDataManager::Get(this);
    if (DM)
    {
        FSMWaveData WaveData = DM->GetWaveData(WaveIndex);
        if (!WaveData.SpawnList.IsEmpty())
            SpawnInterval = WaveData.SpawnList[0].SpawnInterval;
    }

    GetWorld()->GetTimerManager().SetTimer(
        ActivateTimerHandle,
        this,
        &USMWaveManagerSubsystem::TickActivation,
        SpawnInterval,
        true
    );
    
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] Wave %d 전투 시작 - %.1f초 간격 활성화"),
        WaveIndex, SpawnInterval);
    
}

void USMWaveManagerSubsystem::OnMonsterDied(ASMMonsterBase* Monster)
{
    if (GetWorld()->GetNetMode() == NM_Client) return;
    
    AliveMonsters.Remove(Monster);
    ScheduleDestroy(Monster);
    
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] 몬스터 사망 - 생존 : %d"), AliveMonsters.Num());

    CheckWaveCleared();
}

void USMWaveManagerSubsystem::OnClientLoadComplete()
{
    ReadyClientCount++;
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] 클라이언트 로드 완료 (%d / %d)"),
        ReadyClientCount, TotalExpectedClients);
    CheckAllReady();
}

void USMWaveManagerSubsystem::CheckAllReady()
{
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] CheckAllReady - ServerReady: %d, ReadyClientCount: %d / %d"),
        bServerReady, ReadyClientCount, TotalExpectedClients);
    if (bServerReady && ReadyClientCount >= TotalExpectedClients)
    {
        UE_LOG(LogTemp, Log, TEXT("[WaveManager] 서버 + 클라 모두 로드 완료"));
        GetWorld()->GetTimerManager().SetTimer(
            PreSpawnTimerHandle,this,
            &USMWaveManagerSubsystem::TickPreSpawning,0.1f,true);
    }
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

void USMWaveManagerSubsystem::TickDestroy()
{
    if (PendingDestroyActors.IsEmpty())
    {
        GetWorld()->GetTimerManager().ClearTimer(DestroyTimerHandle);
        UE_LOG(LogTemp, Log, TEXT("[WaveManager] TickDestroy 완료 - 모두 제거됨"));
        return;
    }
    
    ASMMonsterBase* Monster = PendingDestroyActors[0];
    PendingDestroyActors.RemoveAt(0);
    
    if (!IsValid(Monster)) return;
    
    //Controller 정리 후 Actor Destroy
    if (AController* Ctrl = Monster->GetController())
    {
        Ctrl->UnPossess();
        Ctrl->Destroy();
    }
    Monster->Destroy();
    
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] Actor Destroy - 대기: %d"), PendingDestroyActors.Num());
}

void USMWaveManagerSubsystem::ScheduleDestroy(ASMMonsterBase* Monster)
{
    if (!IsValid(Monster)) return;
    
    PendingDestroyActors.Add(Monster);
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] ScheduleDestroy 등록 - 대기: %d"), PendingDestroyActors.Num());
    if (!GetWorld()->GetTimerManager().IsTimerActive(DestroyTimerHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(
            DestroyTimerHandle,
            this,
            &USMWaveManagerSubsystem::TickDestroy,
            0.1f,
            true
            );
    }
}

ASMMonsterSpawner* USMWaveManagerSubsystem::GetRandomSpawner() const
{
    if (Spawners.IsEmpty()) return nullptr;

    int32 RandomIndex = FMath::RandRange(0, Spawners.Num() - 1);
    return Spawners[RandomIndex];
}

void USMWaveManagerSubsystem::CheckWaveCleared()
{
    // Activation 타이머가 아직 살아있으면 (PreSpawnedActors가 남아있을 수 있음)
    if (GetWorld()->GetTimerManager().IsTimerActive(ActivateTimerHandle)) return;
    
    //PreSpawnedActors가 남이있으면
    if (!PreSpawnedActors.IsEmpty()) return;
    
    //살아있는 몬스터가 있으면
    if (!AliveMonsters.IsEmpty()) return;
    
    UE_LOG(LogTemp, Log, TEXT("[WaveManager] 웨이브 클리어!"));

    if(ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>())
        GM->OnWaveCleared.ExecuteIfBound();
}
