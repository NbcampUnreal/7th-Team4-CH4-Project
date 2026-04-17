#include "SMBaseBuilding.h"

#include "AbilitySystemComponent.h"
#include "SagoMagic.h"
#include "SMGridManager.h"
#include "GAS/AttributeSets/SMBuildingAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NavModifierComponent.h"
#include "Components/SMInteractionTargetComponent.h"
#include "GameFramework/PlayerState.h"
#include "GAS/AttributeSets/SMBaseCampAttributeSet.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "NavAreas/NavArea_Default.h"
#include "NavAreas/NavArea_Obstacle.h" 


ASMBaseBuilding::ASMBaseBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// StaticMesh가 NavMesh를 차단하지 않도록 설정
	MeshComponent->SetCanEverAffectNavigation(false);

	// NavModifier로 통과 가능하되 비용이 높은 영역으로 설정
	NavModifierComp = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	//NavModifierComp->SetupAttachment(RootComponent);
	NavModifierComp->SetAreaClass(UNavArea_Default::StaticClass());

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(
		EGameplayEffectReplicationMode::Minimal);
	SetReplicatingMovement(true);	
	AttributeSet = CreateDefaultSubobject<USMBuildingAttributeSet>(TEXT("AttributeSet"));
	
	InteractionTargetComponent = CreateDefaultSubobject<USMInteractionTargetComponent>(TEXT("InteractionTargetComp"));
	InteractionTargetComponent->SetInteractionDisplayText(FText::FromString(TEXT("수리")));
}

void ASMBaseBuilding::HandleInteract(APawn* InInteractingPawn)
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

void ASMBaseBuilding::BeginPlay()
{
	Super::BeginPlay();
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

UAbilitySystemComponent* ASMBaseBuilding::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASMBaseBuilding::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASMBaseBuilding, GridPos);
	DOREPLIFETIME(ASMBaseBuilding, bIsDestructible);
	DOREPLIFETIME(ASMBaseBuilding, bIsDead);
}

void ASMBaseBuilding::InitBuilding(FIntPoint InGridPos, bool bInIsDestructible, float InMaxHealth)
{
	GridPos = InGridPos;
	bIsDestructible = bInIsDestructible;

	if (HasAuthority() && AttributeSet)
	{
		AttributeSet ->InitMaxHealth(InMaxHealth);
		AttributeSet->InitHealth(InMaxHealth);
	}
}

void ASMBaseBuilding::HandleDestruction_Implementation()
{
	if (!HasAuthority() || !bIsDestructible || bIsDead) return;
	
	bIsDead = true;

	if (ASMGridManager* GM = GetGridManager())
	{
		GM->ClearCellsByActor(this);
		SM_LOG(this, LogSM, Log, TEXT("[BaseBuilding] 파괴 - GridPos(%d, %d)"), GridPos.X, GridPos.Y);
	}
	Destroy();
	//SetLifeSpan(2.f);
}

float ASMBaseBuilding::GetMaxHealth()
{
	return AttributeSet ? AttributeSet->GetMaxHealth() : 0.f;
}

float ASMBaseBuilding::GetCurrentHealth() const
{
	return AttributeSet ? AttributeSet->GetHealth() : 0.f;
}

void ASMBaseBuilding::OnRep_IsDead()
{
	if (bIsDead)
	{
		//이펙트, 사운드 재생??
		SM_LOG(this, LogSM, Log, TEXT("[BaseBuilding] 클라이언트 파괴 연출"));
	}
}

ASMGridManager* ASMBaseBuilding::GetGridManager() const
{
	return Cast<ASMGridManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ASMGridManager::StaticClass()));
}

