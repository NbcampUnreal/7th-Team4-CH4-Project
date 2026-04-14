// SMPlayerCharacter.h

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "SMPlayerCharacter.generated.h"

struct FOnAttributeChangeData;
struct FInputActionValue;
class USMInteractionScannerComponent;
class USMAbilitySystemComponent;
class UGameplayAbility;
class USMPlayerAttributeSet;
class UAbilitySystemComponent;
class UInputMappingContext;
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
	TObjectPtr<UInputMappingContext> DefaultIMC;
	
	// TODO: 건설관련 논의 끝난 후 에디터에서 IMC 설정 필요
	UPROPERTY(EditAnywhere, Category = "Input|Build|Place")
	TObjectPtr<UInputMappingContext> BuildPlaceIMC;
	
	// TODO: 건설관련 논의 끝난 후 에디터에서 IMC 설정 필요
	UPROPERTY(EditAnywhere, Category = "Input|Build|Edit")
	TObjectPtr<UInputMappingContext> BuildEditIMC;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> AttackAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;
	
	UPROPERTY(EditAnywhere, Category = "Input|Build|Place")
	TObjectPtr<UInputAction> BuildAction;
	
	UPROPERTY(EditAnywhere, Category = "Input|Build|Edit")
	TObjectPtr<UInputAction> EditAction;
	
	UPROPERTY(EditAnywhere, Category = "Input|Build|Place")
	TObjectPtr<UInputAction> BuildPlaceAction;
	
	UPROPERTY(EditAnywhere, Category = "Input|Build|Edit")
	TObjectPtr<UInputAction> EditSelectAction;
	
	UPROPERTY(EditAnywhere, Category = "Input|Quickslot")
	TObjectPtr<UInputAction> QuickSlotAction;

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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

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
	
	/** 죽음 판정 플래그 */
	UPROPERTY(BlueprintReadOnly, Category = "Death", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathLifeSpan = 5.0f;
	
	/** 서버에서 bIsDead가 바뀌면 클라에게 복제해줄 함수 */
	UFUNCTION()
	virtual void OnRep_IsDead();
	
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	
	void HandleDeath();

public:
	/** Adds inputs bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	virtual void PawnClientRestart() override;

protected:
	void Move(const FInputActionValue& Value);
	
	void Attack();
	
	void Interact();
	
	void UseQuickSlot(const FInputActionValue& InValue);
	
	// B버튼 및 V버튼을 누르면 모드가 바뀌는 함수
	void ToggleBuildMode();
	void ToggleEditMode();
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetBuildModeTag(bool bEnable);

	UFUNCTION(Server, Reliable)
	void ServerRPC_SetEditModeTag(bool bEnable);
	
	// 건축모드나 편집 모드일시 좌클릭 누르면 실행될 함수
	void OnBuildPlace();
	void OnEditSelect();
	
	//================================
	// 캐릭터 커스터마이징
	//================================
public:
	/** 커스터마이즈 모드 진입/죵료 (이동 잠금 + 카메라 전환) 로컬 전용 */
	void SetCustomizeMode(bool bEnable);

	/** PlayerState 인덱스 읽어서 무기/머티리얼 즉시 적용. 로컬 미리보기용 */
	void ApplyCustomizationLocal(int32 WeaponIndex, int32 MaterialIndex);

	/** PlayerState 인덱스 읽어서 무기/머티리얼 적용. OnRep_ 및 스폰 시 호출 */
	void ApplyCustomization();
	
	/** 머티리얼 옵션 개수 반환 - SMCustomizeWidget 순환 계산용 */
	int32 GetMaterialOptionNum() const { return MaterialOptions.Num(); }
	
protected:
	/** 커스터마이즈 뷰 카메라 거리 (Blueprint에서 조정 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "Customization|Camera")
	float CustomizeCameraLength = 450.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Customization|Camera")
	FRotator CustomizeCameraRotation = FRotator(-15.0f, 180.0f, 0.0f);
	
private:
	bool bIsInCustomizeMode = false;
	
	/** 무기 메시 옵션 배열 - Blueprint에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TArray<TObjectPtr<UStaticMesh>> WeaponMeshOptions;
	
	/** 머티리얼 옵션 배열 - Blueprint에서 할당 */
	UPROPERTY(EditDefaultsOnly, Category = "Customization")
	TArray<TObjectPtr<UMaterialInterface>> MaterialOptions;
	
};
