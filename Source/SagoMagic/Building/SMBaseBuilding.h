#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "SMBaseBuilding.generated.h"

class ASMGridManager;
class USMBuildingAttributeSet;
/**
 * 모든 건물의 베이스 클래스
 */
UCLASS()
class SAGOMAGIC_API ASMBaseBuilding : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASMBaseBuilding();

	virtual  UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "Building")
	void InitBuilding(FIntPoint InGridPos, bool bInIsDestructible, float InMaxHealth);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Building")
	void HandleDestruction();
	
	UFUNCTION(BlueprintPure, Category = "Building")
	float GetMaxHealth();
	
	UFUNCTION(BlueprintPure, Category ="Building")
	float GetCurrentHealth() const;
	
	UFUNCTION(BlueprintPure, Category = "Building")
	FIntPoint GetGridPos() const { return GridPos; }
	
	UFUNCTION(BlueprintPure, Category = "Building")
	bool GetIsDestructible() const { return bIsDestructible; }
protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION()
	void OnRep_IsDead();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<USMBuildingAttributeSet> AttributeSet;
	
	UPROPERTY(Replicated)
	FIntPoint GridPos = FIntPoint(0, 0);
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Building")
	bool bIsDestructible = true;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;
private:
	ASMGridManager* GetGridManager() const;
	
};
