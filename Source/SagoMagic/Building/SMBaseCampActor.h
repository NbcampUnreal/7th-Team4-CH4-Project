#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/AttributeSets/SMBaseCampAttributeSet.h"
#include "SMBaseCampActor.generated.h"

class APawn;
class USMInteractionTargetComponent;

UCLASS()
class SAGOMAGIC_API ASMBaseCampActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ASMBaseCampActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	float GetCurrentHealth() const;
	
	/** 구조물 수리 요청 */
	UFUNCTION(BlueprintCallable, Category="Interact|Repair")
	void HandleInteract(APawn* InInteractingPawn);
	
protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<USMBaseCampAttributeSet> AttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<USMInteractionTargetComponent> InteractionTargetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Repair")
	float RepairCost = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction|Repair")
	float RepairAmount = 5.0f;
};
