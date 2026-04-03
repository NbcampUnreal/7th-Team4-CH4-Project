// SMLobbyGameState.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Core/SessionSubsystem/SMPlayerSlotInfo.h"
#include "SMLobbyGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSMplayerSlotChanged);

/**
 * 로비 게임 스테이트
 */
UCLASS()
class SAGOMAGIC_API ASMLobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
	//ASMLobbyGameMode가 호출 (서버only)
	void UpdatePlayerSlots(const TArray<FSMPlayerSlotInfo>& NewSlots);

	//USMLobbyWidget이 읽는 함수
	const TArray<FSMPlayerSlotInfo>& GetPlayerSlots() const { return PlayerSlots; }

	//USMLobbyWidget에 바인딩하는 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnSMplayerSlotChanged OnPlayerSlotChanged;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerSlots)
	TArray<FSMPlayerSlotInfo> PlayerSlots;

	UFUNCTION()
	void OnRep_PlayerSlots();
};
