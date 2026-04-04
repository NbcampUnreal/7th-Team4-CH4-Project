// SMInteractionScannerComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "SMInteractionScannerComponent.generated.h"

class USMInteractionTargetComponent;
/**
 * 플레이어에게 부착되어 주변의 상호작용 가능한 대상을 스캔하고
 * 가장 가까운 대상에게 하이라이트 시키는 컴포넌트
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMInteractionScannerComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	USMInteractionScannerComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	USMInteractionTargetComponent* GetClosestTarget() const { return CurrentFocusedTarget; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnScannerBeginOverlap(
		UPrimitiveComponent* OverlapComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSeep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnScannerEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
public:
	USMInteractionTargetComponent* FindClosestTarget() const;
	
	UPROPERTY(EditAnywhere, Category = "Interaction|Debug")
	bool bShowDebug;

private:
	/** 댕글링 포인터를 막기 위한 리플렉션 */
	UPROPERTY()
	TArray<TObjectPtr<USMInteractionTargetComponent>> OverlappedTargets;

	/** 댕글링 포인터를 막기 위한 리플렉션 */
	UPROPERTY()
	TObjectPtr<USMInteractionTargetComponent> CurrentFocusedTarget;
};
