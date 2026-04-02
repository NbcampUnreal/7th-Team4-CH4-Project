#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Wave/IGamePhaseCommand.h"
#include "SMGameFlowSubSystem.generated.h"

/**
 * GameFlow를 CommandQueue로 관리하는 subsystem
 */
UCLASS()
class SAGOMAGIC_API USMGameFlowSubSystem : public UWorldSubsystem,public FTickableGameObject
{
    GENERATED_BODY()
public:
    //interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
private:
    void AdvanceCommand();
private:
    /** 현재 command */
    TSharedPtr<IGamePhaseCommand> CurrentCommand;
    /** Game flow를 담당할 queue */
    TQueue<TSharedPtr<IGamePhaseCommand>> CommandQueue;




};
