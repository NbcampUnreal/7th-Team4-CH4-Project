#include "TestMonsterSpawner.h"
#include "Enemy/SMMonsterBase.h"
#include "Data/SMMonsterDataAsset.h"
#include "TimerManager.h"

ATestMonsterSpawner::ATestMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void ATestMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (!HasAuthority()) return;

    if (!MonsterDataAsset || MonsterDataAsset->MonsterClass.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("[TestMonsterSpawner] DataAsset 또는 MonsterClass 미설정"));
        return;
    }

    GetWorldTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &ATestMonsterSpawner::SpawnMonster,
        SpawnInterval,
        true
    );
}

void ATestMonsterSpawner::SpawnMonster()
{
    if (!MonsterDataAsset) return;

    if (MaxSpawnCount > 0 && CurrentSpawnCount >= MaxSpawnCount)
    {
        GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    // SoftClassPtr → 동기 로드 (테스트용)
    TSubclassOf<ASMMonsterBase> MonsterClass = MonsterDataAsset->MonsterClass.LoadSynchronous();
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TestMonsterSpawner] MonsterClass 로드 실패"));
        return;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    ASMMonsterBase* Monster = GetWorld()->SpawnActor<ASMMonsterBase>(
        MonsterClass,
        GetActorLocation(),
        GetActorRotation(),
        Params
    );
    if (!Monster) return;

    // L_Play → OnRep_MonsterAssetId로 AsyncDataManager 통해 복제
    // 그 외 맵 → 아래 MulticastApplyVisuals로 직접 처리
    Monster->MonsterAssetId = MonsterDataAsset->GetPrimaryAssetId();

    // 서버 + 클라 모두 비주얼 적용
    MulticastApplyVisuals(Monster);

    // AutoPossessAI = Disabled 이므로 수동으로 AI 시작
    Monster->SpawnDefaultController();

    CurrentSpawnCount++;
    UE_LOG(LogTemp, Log, TEXT("[TestMonsterSpawner] 스폰 완료 (%d/%s)"),
        CurrentSpawnCount,
        MaxSpawnCount > 0 ? *FString::FromInt(MaxSpawnCount) : TEXT("∞"));
}

void ATestMonsterSpawner::MulticastApplyVisuals_Implementation(ASMMonsterBase* Monster)
{
    if (!IsValid(Monster) || !MonsterDataAsset) return;
    Monster->ApplyVisuals(MonsterDataAsset);
}
