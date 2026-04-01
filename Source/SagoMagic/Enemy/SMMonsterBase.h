#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "SMMonsterBase.generated.h"

UCLASS()
class SAGOMAGIC_API ASMMonsterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ASMMonsterBase();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
    UPROPERTY(ReplicatedUsing = OnRepHealth, BlueprintReadOnly, Category = "Monster")
    float Health;

    UFUNCTION()
    void OnRepHealth(); // 체력이 변했을 때 클라이언트에서 실행될 시각적 효과(UI 업데이트 등)
};

