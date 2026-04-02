// SMPlayerCharacter.cpp


#include "SMPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SMPlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
}

void ASMPlayerCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpringArmComp)
	{
		SpringArmComp->SetRelativeRotation(FRotator(-CameraAngle, 0.0f, 0.0f));
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

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
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

void ASMPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		}
	}
}
