// SMPlayerCharacter.cpp


#include "SMPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMInteractionScannerComponent.h"
#include "SMPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Core/SMPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayTags/Character/SMCharacterTag.h"
#include "GameplayTags/Character/SMSkillTag.h"

ASMPlayerCharacter::ASMPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 네트워크 설정
	bReplicates = true;
	SetReplicateMovement(true);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);

	SpringArmComp->TargetArmLength = CameraLength;
	SpringArmComp->bDoCollisionTest = false;
	SpringArmComp->bUsePawnControlRotation = false;

	SpringArmComp->SetUsingAbsoluteRotation(true);
	SpringArmComp->bInheritPitch = false;
	SpringArmComp->bInheritYaw = false;
	SpringArmComp->bInheritRoll = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp);

	// 캐릭터의 움직임으로 몸 회전 금지
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// 컨트롤러 입력으로만 몸 회전
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	InteractionScannerComp = CreateDefaultSubobject<USMInteractionScannerComponent>(TEXT("InteractionScanner"));
	InteractionScannerComp->SetupAttachment(RootComponent);
}

UAbilitySystemComponent* ASMPlayerCharacter::GetAbilitySystemComponent() const
{
	if (SMAbilitySystemComponent)
	{
		return SMAbilitySystemComponent;
	}
	
	if (const ASMPlayerState* PS = GetPlayerState<ASMPlayerState>())
	{
		return PS->GetSMAbilitySystemComponent();
	}
	
	return nullptr;
}

USMAbilitySystemComponent* ASMPlayerCharacter::GetSMAbilitySystemComponent() const
{
	return SMAbilitySystemComponent;
}

USMPlayerAttributeSet* ASMPlayerCharacter::GetAttributeSet() const
{
	if (AttributeSet)
	{
		return AttributeSet;
	}
	
	if (const ASMPlayerState* PS = GetPlayerState<ASMPlayerState>())
	{
		return PS->GetAttributeSet();
	}
	
	return nullptr;
}

void ASMPlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpringArmComp)
	{
		SpringArmComp->SetRelativeRotation(FRotator(-CameraAngle, 0.0f, 0.0f));
		SpringArmComp->TargetArmLength = CameraLength;
	}
}

void ASMPlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(FVector::ForwardVector, MovementVector.X);
		AddMovementInput(FVector::RightVector, MovementVector.Y);
	}
}

void ASMPlayerCharacter::Attack()
{
	if (!SMAbilitySystemComponent) return;
	
	FGameplayTag AttackTag = SMSkillTag::Ability_Skill;
	SMAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(AttackTag));
}

void ASMPlayerCharacter::Interact()
{
	if (!SMAbilitySystemComponent) return;
	
	FGameplayTag InteractTag = SMCharacterTag::Ability_Default_Interact;
	SMAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(InteractTag));
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASMPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 로컬만 틱에서 실행
	if (IsLocallyControlled() && Controller)
	{
		if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
		{
			FHitResult Hit;

			// TODO: 채널 설정을 바닥으로만 할 필요 있음
			bool bHit = PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit);

			if (bHit)
			{
				FVector LookDirection = Hit.Location - GetActorLocation();

				LookDirection.Z = 0.0f;

				if (!LookDirection.IsNearlyZero())
				{
					FRotator NewTargetRotation = LookDirection.Rotation();
					PC->SetControlRotation(NewTargetRotation);
				}
			}
		}
	}
}

void ASMPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	// 서버에서 호출
	InitializeAbilitySystem();
	GiveDefaultAbilities();
}

void ASMPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
	// 클라에서 호출
	// Ability 부여는 서버에서만(클라는 복제)
	InitializeAbilitySystem();
}

void ASMPlayerCharacter::InitializeAbilitySystem()
{
	ASMPlayerState* PS = GetPlayerState<ASMPlayerState>();
	
	if (!PS)
	{
		return;
	}
	
	SMAbilitySystemComponent = PS->GetSMAbilitySystemComponent();
	AttributeSet = PS->GetAttributeSet();
	
	if (SMAbilitySystemComponent)
	{
		// Owner는 PlayerState
		SMAbilitySystemComponent->InitAbilityActorInfo(PS, this);

		UE_LOG(LogTemp, Log, TEXT("[%s] SMASC initialized from PlayerState"), *GetName());
	}
}

void ASMPlayerCharacter::GiveDefaultAbilities()
{
	if (!SMAbilitySystemComponent)
	{
		return;
	}
	
	// 서버에서만 부여
	if (!HasAuthority())
	{
		return;;
	}
	
	for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			// Ability Spec 생성
			// - InputID 없음 (나중에 Input Binding에서 설정)
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			SMAbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	// 캐릭터가 생성되기 전에 PlayerState 데이터가 서버로부터 매우 빠르게 날아올 수 있으므로
	// 한 번더 이니셜라이즈(Lyra도 총 3번 호출 함)
	InitializeAbilitySystem();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
		{
			if (PC->IsLocalController() && DefaultMappingContext)
			{
				if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
							ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
					{
						Subsystem->RemoveMappingContext(DefaultMappingContext);
						Subsystem->AddMappingContext(DefaultMappingContext, 0);
					}
				}
			}
		}

		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		}
		
		if (AttackAction)
		{
			EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
		}
		
		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);
		}
	}
}
