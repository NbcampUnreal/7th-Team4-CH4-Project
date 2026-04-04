#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMMonsterSpawner.generated.h"

class UBoxComponent;
/**
 * 레벨에 배치되는 스폰 액터
 * WaveManager가 SpawnMonster()를 호출해 몬스터를 소환
 */
UCLASS()
class SAGOMAGIC_API ASMMonsterSpawner : public AActor
{
    GENERATED_BODY()
public:
    ASMMonsterSpawner();
protected:
    virtual void BeginPlay() override;
public:
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> Scene = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UBoxComponent> SpawnBox = nullptr;

    UFUNCTION()
    FVector GetRandomPointInVolume() const;

    /**
     *지정 클래스의 몬스터를 자신의 위치에 스폰
     */
    APawn* SpawnMonster(TSubclassOf<APawn> MonsterClass);
};
