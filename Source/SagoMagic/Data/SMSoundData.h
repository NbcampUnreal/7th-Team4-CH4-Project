#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SMSoundData.generated.h"

class USoundBase;
class USoundConcurrency;

UENUM(BlueprintType)
enum class ESMSoundCategory : uint8
{
	NONE,
	BGM,
	SFX,
	UI
};

USTRUCT(BlueprintType)
struct FSMSoundData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName SoundName;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> SoundAsset;
	
	UPROPERTY(EditAnywhere)
	float VolumeMultiplier = 1.0f;
	
	UPROPERTY(EditAnywhere)
	ESMSoundCategory Category = ESMSoundCategory::NONE;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundConcurrency> ConcurrencySettings;
};
