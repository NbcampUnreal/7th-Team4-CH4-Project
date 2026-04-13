#pragma once

#include "CoreMinimal.h"
#include "SMBaseBuilding.h"
#include "SMFenceBuilding.generated.h"

UCLASS()
class SAGOMAGIC_API ASMFenceBuilding : public ASMBaseBuilding
{
	GENERATED_BODY()

public:
	ASMFenceBuilding();

	UFUNCTION(BlueprintCallable, Category = "Building|Fence")
	void ConvertToCorner(float Yaw);
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UFUNCTION()
	void OnRep_bIsCorner();
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Fence|Mesh")
	TObjectPtr<UStaticMesh> StraightMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building|Fence|Mesh")
	TObjectPtr<UStaticMesh> CornerMesh;
private:
	UPROPERTY(ReplicatedUsing = OnRep_bIsCorner)
	bool bIsCorner = false;
	
	UPROPERTY(Replicated)
	float CornerYaw = 0.f;
	
};
