#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GAS/AttributeSets/SMBaseCampAttributeSet.h"
#include "SMBaseCampActor.generated.h"

UCLASS()
class SAGOMAGIC_API ASMBaseCampActor : public AActor,public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ASMBaseCampActor();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<USMBaseCampAttributeSet> AttributeSet;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
};
