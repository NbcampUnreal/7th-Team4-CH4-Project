#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMDamageFloatingText.generated.h"

UCLASS()
class SAGOMAGIC_API ASMDamageFloatingText : public AActor
{
	GENERATED_BODY()

public:	
	ASMDamageFloatingText();

protected:
	virtual void BeginPlay() override;

public:	
	/** 위젯을 화면에 띄우기 위한 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	class UWidgetComponent* DamageWidgetComp;
	/** 액터가 파괴될 시간 */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float LifeSpan = 1.0f;

	/** 데미지 수치를 설정하는 함수 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetDamageValue(float DamageAmount);
	
private:
	float PendingDamage = 0.f;
};
