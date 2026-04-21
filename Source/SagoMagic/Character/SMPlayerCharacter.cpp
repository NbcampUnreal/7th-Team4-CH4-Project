// SMPlayerCharacter.cpp


#include "SMPlayerCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SagoMagic.h"
#include "SMInteractionScannerComponent.h"
#include "SMPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/BuildingModeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SMEditModeComponent.h"
#include "Core/SMGameMode.h"
#include "Core/SMPlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameplayTags/Character/SMCharacterTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Net/UnrealNetwork.h"

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
	
	// InteractionScanner는 공격 못하게 방어
	
	// 물리 X, 오버랩만 판정
	InteractionScannerComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 모든 채널 무시
	InteractionScannerComp->SetCollisionResponseToChannels(ECR_Ignore);
	// 인터렉션만 오버랩 허용
	InteractionScannerComp->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
	
	BuildingModeComp = CreateDefaultSubobject<USMBuildingModeComponent>(TEXT("BuildingModeComponent"));
	EditModeComp = CreateDefaultSubobject<USMEditModeComponent>(TEXT("EditModeComponent"));
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
	if (bIsInCustomizeMode) return;

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

	if (ASMPlayerState* PS = GetPlayerState<ASMPlayerState>())
	{
		if (USMInventoryComponent* InventoryComp = PS->GetInventoryComponent())
		{
			FGameplayTag ActiveSkillTag = InventoryComp->GetActiveSkillTag();

			if (ActiveSkillTag.IsValid())
			{
				SMAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(ActiveSkillTag));

				SM_LOG(this, LogSM, Log, TEXT("%s 마법 발동"), *ActiveSkillTag.ToString());
			}
		}
	}
}

void ASMPlayerCharacter::AttackReleased()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	BroadcastAttackReleasedEvent();

	if (!HasAuthority())
	{
		ServerRPC_NotifyAttackReleased();
	}
}

void ASMPlayerCharacter::Interact()
{
	if (!SMAbilitySystemComponent) return;

	FGameplayTag InteractTag = SMCharacterTag::Ability_Default_Interact;
	SMAbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(InteractTag));
}

void ASMPlayerCharacter::UseQuickSlot(const FInputActionValue& InValue)
{
	if (!IsLocallyControlled()) return;

	const int32 SlotIndex = FMath::RoundToInt(InValue.Get<float>() - 1);

	if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
	{
		PC->ServerRPCSetActiveQuickSlot(SlotIndex);
		SM_LOG(this, LogSM, Log, TEXT("퀵슬롯 %d번 장착"), SlotIndex);
	}
}

void ASMPlayerCharacter::ToggleBuildMode()
{
	ASMPlayerController* PC = Cast<ASMPlayerController>(Controller);
	if (!PC || !PC->IsLocalController()) return;
	if (!SMAbilitySystemComponent || !BuildingModeComp) return;
	
	bool bIsBuildMode = SMAbilitySystemComponent->HasMatchingGameplayTag(SMCharacterTag::State_Build_Place);
	bool bIsEditMode = SMAbilitySystemComponent->HasMatchingGameplayTag(SMCharacterTag::State_Build_Edit);

	// 건축 모드에서 B입력 시 건축 모드 종료
	if (bIsBuildMode)
	{
		BuildingModeComp->DisableBuildMode();
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Place);
		ServerRPC_SetBuildModeTag(false);

		SM_LOG(this, LogSM, Log, TEXT("건축 모드 종료"));
		return;
	}

	// 편집모드라면 편집모드 종료
	if (bIsEditMode)
	{
		EditModeComp->DisableEditMode();
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Edit);
		ServerRPC_SetEditModeTag(false);
	}

	// 건축모드 켜기
	BuildingModeComp->EnableBuildMode();
	SMAbilitySystemComponent->AddLooseGameplayTag(SMCharacterTag::State_Build_Place);
	ServerRPC_SetBuildModeTag(true);
	
	SM_LOG(this, LogSM, Log, TEXT("건축 모드 ON"));
}

void ASMPlayerCharacter::ToggleEditMode()
{
	ASMPlayerController* PC = Cast<ASMPlayerController>(Controller);
	if (!PC || !PC->IsLocalController()) return;

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem || !SMAbilitySystemComponent) return;

	bool bIsBuildMode = SMAbilitySystemComponent->HasMatchingGameplayTag(SMCharacterTag::State_Build_Place);
	bool bIsEditMode = SMAbilitySystemComponent->HasMatchingGameplayTag(SMCharacterTag::State_Build_Edit);

	// 이미 편집 모드라면 편집모드 종료
	if (bIsEditMode)
	{
		EditModeComp->DisableEditMode();
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Edit);
		ServerRPC_SetEditModeTag(false);

		SM_LOG(this, LogSM, Log, TEXT("편집 모드 OFF"));
		return;
	}

	// 건축 모드라면 건축모드 종료
	if (bIsBuildMode)
	{
		BuildingModeComp->DisableBuildMode();
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Place);
		ServerRPC_SetBuildModeTag(false);
	}

	EditModeComp->EnableEditMode();
	SMAbilitySystemComponent->AddLooseGameplayTag(SMCharacterTag::State_Build_Edit);
	ServerRPC_SetEditModeTag(true);

	SM_LOG(this, LogSM, Log, TEXT("편집 모드 ON"));
}

void ASMPlayerCharacter::ServerRPC_SetBuildModeTag_Implementation(bool bEnable)
{
	if (!SMAbilitySystemComponent) return;

	if (bEnable)
	{
		SMAbilitySystemComponent->AddLooseGameplayTag(SMCharacterTag::State_Build_Place);
	}
	else
	{
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Place);
	}
}

void ASMPlayerCharacter::ServerRPC_SetEditModeTag_Implementation(bool bEnable)
{
	if (!SMAbilitySystemComponent) return;

	if (bEnable)
	{
		SMAbilitySystemComponent->AddLooseGameplayTag(SMCharacterTag::State_Build_Edit);
	}
	else
	{
		SMAbilitySystemComponent->RemoveLooseGameplayTag(SMCharacterTag::State_Build_Edit);
	}
}

void ASMPlayerCharacter::ServerRPC_NotifyAttackReleased_Implementation()
{
	BroadcastAttackReleasedEvent();
}

void ASMPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASMPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMPlayerCharacter, bIsDead);
}

void ASMPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 로컬만 틱에서 실행
	if (!bIsDead && !bIsInCustomizeMode && IsLocallyControlled() && Controller)
	{
		if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
		{
			if (PC->IsMoveInputIgnored())
			{
				return;
			}

			FHitResult Hit;

			// Ground 채널만 처리
			bool bHit = PC->GetHitResultUnderCursor(ECC_GameTraceChannel1, true, Hit);

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

	// 이속 핵 방어 — 서버가 기준 이속 캐싱 후 1초마다 검증
	if (HasAuthority())
	{
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			AuthorizedMaxWalkSpeed = MoveComp->MaxWalkSpeed;
		}

		GetWorldTimerManager().SetTimer(
			SpeedCheckTimerHandle,
			this,
			&ASMPlayerCharacter::ServerValidateMovementSpeed,
			1.0f,
			true);
	}
}

void ASMPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// 클라에서 호출
	// Ability 부여는 서버에서만(클라는 복제)
	InitializeAbilitySystem();
	ApplyCustomization();
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

	if (SMAbilitySystemComponent && AttributeSet)
	{
		// Owner는 PlayerState
		SMAbilitySystemComponent->InitAbilityActorInfo(PS, this);
		
		if (!SMAbilitySystemComponent->HasMatchingGameplayTag(SMGameFlowTag::Team_Player))
		{
			SMAbilitySystemComponent->AddLooseGameplayTag(SMGameFlowTag::Team_Player);
		}

		SMAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()).RemoveAll(this);

		// SMASC로부터 플레이어의 체력변화 구독
		SMAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			AttributeSet->GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);

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
		if (!AbilityClass) continue;
		
		// 이미 부여된 Spec이 있으면 스킵
		if (SMAbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass)) continue;
		
		// Ability Spec 생성
		// - InputID 없음 (나중에 Input Binding에서 설정)
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
		SMAbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void ASMPlayerCharacter::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	// 서버에서만 사망 판정
	if (HasAuthority() && Data.NewValue <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		HandleDeath();
	}
}

void ASMPlayerCharacter::OnRep_IsDead()
{
	// 클라이언트 동기화
	if (bIsDead)
	{
		HandleDeath();
	}
}

void ASMPlayerCharacter::ServerValidateMovementSpeed()
{
	if (!HasAuthority() || bIsDead) return;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp == nullptr) return;

	// MaxWalkSpeed가 기준치의 120%를 초과하면 강제 복구
	if (MoveComp->MaxWalkSpeed > AuthorizedMaxWalkSpeed * 1.2f)
	{
		MoveComp->MaxWalkSpeed = AuthorizedMaxWalkSpeed;
	}
}

void ASMPlayerCharacter::HandleDeath()
{
	SM_LOG(this, LogSM, Log, TEXT("[%s] 플레이어 사망."), *GetName());


	if (SMAbilitySystemComponent)
	{
		SMAbilitySystemComponent->CancelAbilities();
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->DisableMovement();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
	}

	if (HasAuthority())
	{
		if (ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>())
		{
			// GameMode에 사망 통보 - RPC 호출은 GameMode에서 RespawnTime과 함께 처리
			if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
			{
				GM->OnPlayerDead(PC);
			}
		}

		// TODO: DeathLifeSpan후 시체 처리(부활 타이머랑 타이밍 논의 필요)
		SetLifeSpan(DeathLifeSpan);
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
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		}

		if (AttackAction)
		{
			EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::Attack);
			EIC->BindAction(AttackAction, ETriggerEvent::Completed, this, &ThisClass::AttackReleased);
			EIC->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ThisClass::AttackReleased);
		}

		if (InteractAction)
		{
			EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::Interact);
		}

		if (QuickSlotAction)
		{
			EIC->BindAction(QuickSlotAction, ETriggerEvent::Started, this, &ThisClass::UseQuickSlot);
		}
		
		if (BuildModeAction)
		{
			EIC->BindAction(BuildModeAction, ETriggerEvent::Started, this, &ThisClass::ToggleBuildMode);
		}
		
		if (EditModeAction)
		{
			EIC->BindAction(EditModeAction, ETriggerEvent::Started, this, &ThisClass::ToggleEditMode);
		}
		
	}
	if (BuildingModeComp)
	{
		BuildingModeComp->SetupInputBindings();
	}
	if (EditModeComp)
	{
		EditModeComp->SetupInputBindings();
	}
}

void ASMPlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	if (ASMPlayerController* PC = Cast<ASMPlayerController>(Controller))
	{
		if (PC->IsLocalController() && DefaultIMC)
		{
			if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
						ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
				{
					// 기존 IMC 전부 초기화
					if (LobbyIMC) Subsystem->RemoveMappingContext(LobbyIMC);
					Subsystem->RemoveMappingContext(DefaultIMC);

					// 현재 맵에 따라 IMC 결정
					FString MapName = GetWorld()->GetMapName();
					if (LobbyIMC && MapName.Contains(TEXT("Lobby")))
					{
						Subsystem->AddMappingContext(LobbyIMC, 1);
						UE_LOG(LogTemp, Warning, TEXT("[PlayerCharacter] IMC = LobbyIMC"));
					}
					else
					{
						Subsystem->AddMappingContext(DefaultIMC, 0);
						UE_LOG(LogTemp, Warning, TEXT("[PlayerCharacter] IMC = DefaultIMC"));
					}
				}
			}
		}
	}
}

//================================
// 캐릭터 커스터마이징
//================================

void ASMPlayerCharacter::SetCustomizeMode(bool bEnable)
{
	if (IsLocallyControlled() == false) return;
	if (IsValid(SpringArmComp) == false) return;

	UCharacterMovementComponent* MovementComp = GetCharacterMovement();
	if (IsValid(MovementComp) == false) return;

	if (bEnable == true)
	{
		bIsInCustomizeMode = true;
		// 현재 속도(관성) 즉시 제거
		MovementComp->StopMovementImmediately();
		// 이동 잠금
		MovementComp->DisableMovement();

		// 캐릭터 현재 Yaw + 180도 = 캐릭터 정면에서 바라보는 카메라 위치
		float FaceYaw = GetActorRotation().Yaw;
		FRotator CameraRot = FRotator(CustomizeCameraRotation.Pitch, FaceYaw + CustomizeCameraRotation.Yaw, 0.0f);


		SpringArmComp->SetRelativeRotation(CameraRot);
		SpringArmComp->TargetArmLength = CustomizeCameraLength;
	}
	else
	{
		bIsInCustomizeMode = false;

		MovementComp->SetMovementMode(MOVE_Walking);


		SpringArmComp->SetUsingAbsoluteRotation(true);
		SpringArmComp->bInheritPitch = false;
		SpringArmComp->bInheritYaw = false;
		SpringArmComp->bInheritRoll = false;
		SpringArmComp->SetRelativeRotation(FRotator(-CameraAngle, 0.0f, 0.0f));
		SpringArmComp->TargetArmLength = CameraLength;
	}
}

void ASMPlayerCharacter::ApplyCustomizationLocal(int32 WeaponIndex, int32 MaterialIndex)
{
	// Blueprint에서 WeaponSocket에 붙어있는 StaticMeshComponent 탐색
	TArray<UStaticMeshComponent*> Comps;
	GetComponents<UStaticMeshComponent>(Comps);
	for (UStaticMeshComponent* Comp : Comps)
	{
		if (IsValid(Comp) == false) continue;
		if (Comp->GetAttachSocketName() != FName("WeaponSocket")) continue;

		if (WeaponMeshOptions.IsValidIndex(WeaponIndex))
		{
			Comp->SetStaticMesh(WeaponMeshOptions[WeaponIndex]);
		}
		break;
	}

	//캐릭터 스켈레탈 메시 머티리얼 적용
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (IsValid(MeshComp) && MaterialOptions.IsValidIndex(MaterialIndex))
	{
		MeshComp->SetMaterial(0, MaterialOptions[MaterialIndex]);
	}
}

void ASMPlayerCharacter::ApplyCustomization()
{
	ASMPlayerState* PS = GetPlayerState<ASMPlayerState>();
	if (IsValid(PS) == false) return;

	ApplyCustomizationLocal(PS->GetSelectedWeaponIndex(), PS->GetSelectedMaterialIndex());
}

void ASMPlayerCharacter::BroadcastAttackReleasedEvent()
{
	AActor* EventTarget = this;

	if (ASMPlayerState* CurrentPlayerState = GetPlayerState<ASMPlayerState>())
	{
		EventTarget = CurrentPlayerState;
	}

	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.Target = EventTarget;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		EventTarget,
		SMSkillTag::Event_Input_AttackReleased,
		EventData);
}

void ASMPlayerCharacter::ServerSetStaffTipAimOrigin_Implementation(FVector NewOrigin)
{
	ServerStaffTipLocation = NewOrigin;
}
