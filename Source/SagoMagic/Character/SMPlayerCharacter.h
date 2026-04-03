// SMPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SMPlayerCharacter.generated.h"

class USMInteractionScannerComponent;
class USMAbilitySystemComponent;
class UGameplayAbility;
class USMPlayerAttributeSet;
class UAbilitySystemComponent;
class UInputMappingContext;
struct FInputActionValue;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
/**
 * 플레이어가 조종할 캐릭터 클래스
 */
UCLASS()
class SAGOMAGIC_API ASMPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> CameraComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USMInteractionScannerComponent> InteractionScannerComp;
	

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Pitch(상하 각도) 조정용 */
	UPROPERTY(EditAnywhere,
		Category = "Camera",
		meta = (ClampMin = 0.0f, ClampMax = 90.0f, UIMin = 0.0f, UIMax = 90.0f))
	float CameraAngle = 60.0f;

	UPROPERTY(EditAnywhere,
		Category = "Camera",
		meta = (ClampMin = 400.0f, ClampMax = 2000.0f, UIMin = 400.0f, UIMax = 2000.0f))
	float CameraLength = 1800.0f;

public:
	/** Constructor */
	ASMPlayerCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
	USMAbilitySystemComponent* GetSMAbilitySystemComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	USMPlayerAttributeSet* GetAttributeSet() const;
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	USMInteractionScannerComponent* GetInteractionScanner() const { return InteractionScannerComp; }

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;
	
protected:
	virtual void InitializeAbilitySystem();
	
	void GiveDefaultAbilities();
	
	/** PS에서 가져와 캐시된 ASC */
	UPROPERTY()
	TObjectPtr<USMAbilitySystemComponent> SMAbilitySystemComponent;
	
	/** PS에서 가져와 캐시된 AttributeSet */
	UPROPERTY()
	TObjectPtr<USMPlayerAttributeSet> AttributeSet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

public:
	/** Adds inputs bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Move(const FInputActionValue& Value);
	
	void Attack();
	
	void Interact();
};
