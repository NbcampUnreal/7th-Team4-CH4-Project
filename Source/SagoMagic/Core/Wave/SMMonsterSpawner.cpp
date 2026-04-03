#include "SMMonsterSpawner.h"

#include "Components/BoxComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"

ASMMonsterSpawner::ASMMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
    //서버에서만 스폰 - 복제 대상 x
    bReplicates = false;

    Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
    SetRootComponent(Scene);

    SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawningBox"));
    SpawnBox->SetupAttachment(Scene);
}

void ASMMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();
}

FVector ASMMonsterSpawner::GetRandomPointInVolume() const
{
    FVector BoxExtent = SpawnBox->GetScaledBoxExtent();
    FVector BoxOrigin = SpawnBox->GetComponentLocation();

    return BoxOrigin + FVector(
        FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
        FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
        FMath::FRandRange(-BoxExtent.Z,BoxExtent.Z)
    );
}

APawn* ASMMonsterSpawner::SpawnMonster(TSubclassOf<APawn> MonsterClass)
{
    if (!MonsterClass) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* Monster = GetWorld()->SpawnActor<APawn>(
        MonsterClass,
        GetActorLocation(),
        GetActorRotation(),
        Params
        );

    UE_LOG(LogTemp,Log, TEXT("[Spawner] %s에서 %s 스폰"), *GetName(), *GetNameSafe(MonsterClass));
    return Monster;
}


