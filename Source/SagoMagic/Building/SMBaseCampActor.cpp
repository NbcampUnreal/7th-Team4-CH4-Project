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
	
	// 10골드 미만이면 수리 불가
	const float CurrentGold = PlayerAttribute->GetGold();
	if (CurrentGold < 10.0f)
	{
		SM_LOG(this, LogSM, Warning, TEXT("골드가 10G미만입니다."));
		return;
	}
	
	if (!AttributeSet) return;
	const float CurrentHealth = AttributeSet->GetHealth();
	const float MaxHealth = AttributeSet->GetMaxHealth();
	if (CurrentHealth >= MaxHealth) return;
	
	// GE없이 서버에서 직접 차감. PreAttributeChange로직 통과해 클라로 복제
	PlayerASC->SetNumericAttributeBase(USMPlayerAttributeSet::GetGoldAttribute(), CurrentGold - 10.0f);
	AbilitySystemComponent->SetNumericAttributeBase(USMBaseCampAttributeSet::GetHealthAttribute(), CurrentHealth + 10.0f);
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

