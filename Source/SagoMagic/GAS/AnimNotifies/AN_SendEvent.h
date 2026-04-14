// AN_SendEvent.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SendEvent.generated.h"

/**
 * GA_Skill류를 사용할 때 AnimMontage에서 Notify로 Event 태그를 보낼때 필요한 코드
 */
UCLASS()
class SAGOMAGIC_API UAN_SendEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SendEvent();

	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Tag")
	FGameplayTag EventTag;
};
