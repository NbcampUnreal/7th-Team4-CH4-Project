#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_SkillField_Tick.generated.h"

class USoundBase;

UCLASS()
class SAGOMAGIC_API UGCN_SkillField_Tick : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	/** 데미지 틱마다 재생할 사운드 - BP에서 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> TickSound;
};
