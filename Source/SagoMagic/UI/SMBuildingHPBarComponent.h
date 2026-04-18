#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "SMBuildingHPBarComponent.generated.h"

class UAbilitySystemComponent;

/**
 * 건물 위에 표시되는 HP 바 컴포넌트
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class SAGOMAGIC_API USMBuildingHPBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	USMBuildingHPBarComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void TryInitASC();
	void InitializeHPBar(UAbilitySystemComponent* InASC);
	void OnHPChanged(const FOnAttributeChangeData& Data);
	
	/** 데미지를 받아서 표시된 체력바를 숨기는 함수 */
	void HideHPBar();
	/** 플레이어와의 거리를 주기적으로 체크하는 함수 */
	void CheckDistanceToPlayer();
	/** 데미지 상태와 거리 상태를 종합하여 최종적으로 위젯을 띄울지 결정하는 함수 */
	void UpdateVisibility();

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	FTimerHandle HideTimerHandle;
	FTimerHandle ASC_InitTimerHandle;
	FTimerHandle DistanceCheckTimerHandle;
	
	UPROPERTY(EditAnywhere, Category = "UI|Settings")
	float DisplayDuration = 4.0f;
	UPROPERTY(EditAnywhere, Category = "UI|Distance")
	float VisibleDistance = 200.0f;
	UPROPERTY(EditAnywhere, Category = "UI|Distance")
	float DistanceCheckInterval = 0.2f;
	
	bool bIsVisibleFromDamage = false;
	bool bIsVisibleFromProximity = false;
};