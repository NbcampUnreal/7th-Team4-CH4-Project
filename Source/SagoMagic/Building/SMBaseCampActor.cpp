#include "SMBaseCampActor.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "SagoMagic.h"
#include "Components/SMInteractionTargetComponent.h"
#include "Core/SMGameMode.h"
#include "GameFramework/PlayerState.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"

ASMBaseCampActor::ASMBaseCampActor()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<USMBaseCampAttributeSet>(TEXT("AttributeSet"));
	
	InteractionTargetComponent = CreateDefaultSubobject<USMInteractionTargetComponent>(TEXT("InteractionTargetComp"));
	InteractionTargetComponent->SetInteractionDisplayText(FText::FromString(TEXT("수리")));
}

UAbilitySystemComponent* ASMBaseCampActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

float ASMBaseCampActor::GetCurrentHealth() const
{
	if (AttributeSet)
		return AttributeSet->GetHealth();
	return 0.f;
}

void ASMBaseCampActor::HandleInteract(APawn* InInteractingPawn)
{
	if (!HasAuthority()) return;
	
	if (!IsValid(InInteractingPawn)) return;
	
	APlayerState* PS = InInteractingPawn->GetPlayerState();
	if (!PS) return;
	
	UAbilitySystemComponent* PlayerASC = PS->FindComponentByClass<UAbilitySystemComponent>();
	if (!PlayerASC) return;
	
	const USMPlayerAttributeSet* PlayerAttribute = PlayerASC->GetSet<USMPlayerAttributeSet>();
	if (!PlayerAttribute) return;
	
	// RepairCost골드 미만이면 수리 불가
	const float CurrentGold = PlayerAttribute->GetGold();
	if (CurrentGold < RepairCost)
	{
		SM_LOG(this, LogSM, Warning, TEXT("골드가 %.1fG미만입니다."), RepairCost);
		return;
	}
	
	// 현재 체력이 최대 체력이상이면 수리 불가
	if (!AttributeSet) return;
	const float CurrentHealth = AttributeSet->GetHealth();
	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (CurrentHealth >= MaxHealth)
	{
		SM_LOG(this, LogSM, Warning, TEXT("현재 체력이 최대 체력 이상입니다."));
		return;
	}
	
	// GE없이 서버에서 직접 차감. PreAttributeChange로직 통과해 클라로 복제
	PlayerASC->SetNumericAttributeBase(
		USMPlayerAttributeSet::GetGoldAttribute(), CurrentGold - RepairCost);
	AbilitySystemComponent->SetNumericAttributeBase(
		USMBaseCampAttributeSet::GetHealthAttribute(), CurrentHealth + RepairAmount);
	SM_LOG(this, LogSM, Log, TEXT("[수리 성공] 골드: %.1fG 소모, 체력 %.1f 회복"), RepairCost, RepairAmount);
}

void ASMBaseCampActor::BeginPlay()
{
	Super::BeginPlay();
	AbilitySystemComponent->InitAbilityActorInfo(this,this);

	if (HasAuthority())
	{
		if (ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>())
		{
			GM->RegisterBaseCamp(this);
		}
	}
}

