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

{
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
