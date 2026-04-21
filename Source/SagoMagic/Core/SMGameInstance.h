//SMGameInstance.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SMGameInstance.generated.h"

/**
 * 로컬 정보 저장용 GameInstance
 */
UCLASS()
class SAGOMAGIC_API USMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void SetPendingNickname(const FString& Nickname);
	const FString& GetPendingNickname() const { return PendingNickname; }

private:
	FString PendingNickname;
};
