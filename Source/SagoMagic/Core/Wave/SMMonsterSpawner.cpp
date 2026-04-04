#include "SMMonsterSpawner.h"

#include "Components/BoxComponent.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"
#include "GameFramework/Character.h"

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

ACharacter* ASMMonsterSpawner::SpawnMonster(TSubclassOf<ACharacter> MonsterClass)
{
    if (!MonsterClass) return nullptr;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ACharacter* Monster = GetWorld()->SpawnActor<ACharacter>(
        MonsterClass,
        GetActorLocation(),
        GetActorRotation(),
        Params
        );

    UE_LOG(LogTemp,Log, TEXT("[Spawner] %s에서 %s 스폰"), *GetName(), *GetNameSafe(MonsterClass));
    return Monster;
}


