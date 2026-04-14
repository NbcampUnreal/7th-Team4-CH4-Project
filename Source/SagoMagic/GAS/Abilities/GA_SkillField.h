#pragma once

#include "CoreMinimal.h"
#include "GA_SkillBase.h"
#include "SkillActor/SMASkillField.h"
#include "GA_SkillField.generated.h"


UCLASS()
class SAGOMAGIC_API UGA_SkillField : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_SkillField();

protected:
	virtual void OnSkillEffect(
		const FGameplayAbilityActorInfo* ActorInfo,
		const FVector& TargetLocation,
		const FVector& AimDirection) override;

	/** 스폰할 장판 액터 클래스 - BP에서 지정 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	TSubclassOf<ASMASkillField> FieldClass;

	/** 장판 지속 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	float FieldDuration = 5.f;
};
