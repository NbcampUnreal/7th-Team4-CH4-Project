#include "SMEnemyHPBarComponent.h"
#include "UI/SMEnemyHPBarWidget.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "SMDamageFloatingText.h"
#include "TimerManager.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "UObject/ConstructorHelpers.h"


USMEnemyHPBarComponent::USMEnemyHPBarComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawAtDesiredSize(false);

	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultHPBarWidgetClass(
		TEXT("/Game/SagoMagic/UI/Enemy/WBP_EnemyHPBar"));
	if (DefaultHPBarWidgetClass.Succeeded())
	{
		SetWidgetClass(DefaultHPBarWidgetClass.Class);
	}

	static ConstructorHelpers::FClassFinder<ASMDamageFloatingText> DefaultDamageTextClass(
		TEXT("/Game/SagoMagic/UI/Enemy/BP_DamageFloatingText"));
	if (DefaultDamageTextClass.Succeeded())
	{
		DamageTextClass = DefaultDamageTextClass.Class;
	}
}

void USMEnemyHPBarComponent::BeginPlay()
{
	Super::BeginPlay();

	SetVisibility(false);
	TryInitASC();
}

void USMEnemyHPBarComponent::TryInitASC()
{
	if (ASC) return;
	
	if (AActor* OwnerActor = GetOwner())
	{
		// ASC 가지고 있는지 인터페이스로 확인
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(OwnerActor))
		{
			if (UAbilitySystemComponent* OwnerASC = ASI->GetAbilitySystemComponent())
			{
				if (GetUserWidgetObject())
				{
					InitializeHPBar(OwnerASC);
					GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);

					return;
				}
			}
		}
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ASC_InitTimerHandle, this, &USMEnemyHPBarComponent::TryInitASC, 0.1f, false);
	}
}

void USMEnemyHPBarComponent::InitializeHPBar(UAbilitySystemComponent* InASC)
{
	if (InASC)
	{
		ASC = InASC;

		// 중복 바인딩 방지
		ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
		   .RemoveAll(this);

		// ASC 체력 델리게이트 바인딩
		ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
		   .AddUObject(this, &USMEnemyHPBarComponent::OnHPChanged);

		// 초기 HP % 세팅 로직, 내부 데이터는 1.0 상태로 초기화
		USMEnemyHPBarWidget* HPWidget = Cast<USMEnemyHPBarWidget>(GetUserWidgetObject());
		if (HPWidget)
		{
			float CurrentHP = ASC->GetNumericAttribute(USMMonsterAttributeSet::GetHealthAttribute());
			float MaxHP = ASC->GetNumericAttribute(USMMonsterAttributeSet::GetMaxHealthAttribute());

			// MaxHP가 0인 경우 대비 방어 코드
			float HPPercent = (MaxHP > 0.0f) ? (CurrentHP / MaxHP) : 0.0f;

			HPWidget->UpdateHPBar(HPPercent);
		}
	}
}

void USMEnemyHPBarComponent::HandleResolvedDamageNumber(float DamageAmount, const FVector& SpawnLocation)
{
	if (DamageAmount <= 0.0f)
	{
		return;
	}

	RevealHPBar();
	SpawnDamageFloatingText(DamageAmount, SpawnLocation);
}

void USMEnemyHPBarComponent::OnHPChanged(const FOnAttributeChangeData& Data)
{
	RevealHPBar();
	
	if (USMEnemyHPBarWidget* HPWidget = Cast<USMEnemyHPBarWidget>(GetUserWidgetObject()))
	{
		if (ASC)
		{
			float CurrentHP = Data.NewValue;
			float MaxHealth = ASC->GetNumericAttribute(USMMonsterAttributeSet::GetMaxHealthAttribute());
			float HPPercent = (MaxHealth > 0.0f) ? (CurrentHP / MaxHealth) : 0.0f;

			HPWidget->UpdateHPBar(HPPercent);
		}
	}
}

void USMEnemyHPBarComponent::RevealHPBar()
{
	SetVisibility(true);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			HideTimerHandle,
			this,
			&USMEnemyHPBarComponent::HideHPBar,
			DisplayDuration,
			false);
	}
}

void USMEnemyHPBarComponent::HideHPBar()
{
	if (IsValid(this) && IsValid(GetOwner()) && GetWorld())
	{
		SetVisibility(false);
	}
}

void USMEnemyHPBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ASC) // 등록했던 체력 변경 델리게이트 해제
	{
		ASC->GetGameplayAttributeValueChangeDelegate(USMMonsterAttributeSet::GetHealthAttribute())
		   .RemoveAll(this);
	}
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(HideTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(ASC_InitTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void USMEnemyHPBarComponent::SpawnDamageFloatingText(float DamageAmount, const FVector& SpawnLocation)
{
	if (!DamageTextClass) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	UWorld* World = GetWorld();
	if (!World) return;
	
	if (World->GetNetMode() == NM_DedicatedServer) // 데디 서버 스폰 방지
	{
		return;
	}
	
	FVector FinalSpawnLocation = SpawnLocation;
	if (FinalSpawnLocation.IsNearlyZero())
	{
		FinalSpawnLocation = Owner->GetActorLocation() + FVector(0.f, 0.f, 100.f);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASMDamageFloatingText* DamageText = World->SpawnActor<ASMDamageFloatingText>(
		DamageTextClass, FinalSpawnLocation, FRotator::ZeroRotator, Params);

	if (DamageText)
	{
		DamageText->SetDamageValue(DamageAmount);
	}
}
