// SMPlayerState.cpp


#include "SMPlayerState.h"

#include "AbilitySystemComponent.h"
#include "SagoMagic.h"
#include "Character/SMPlayerCharacter.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "Inventory/Components/SMInventoryComponent.h"
#include "Net/UnrealNetwork.h"

ASMPlayerState::ASMPlayerState()
{
	SMAbilitySystemComponent = CreateDefaultSubobject<USMAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	SMAbilitySystemComponent->SetIsReplicated(true);
	SMAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	SMInventoryComponent = CreateDefaultSubobject<USMInventoryComponent>(TEXT("InventoryComponent"));
	SMInventoryComponent->SetIsReplicated(true);


	AttributeSet = CreateDefaultSubobject<USMPlayerAttributeSet>(TEXT("AttributeSet"));

	SetNetUpdateFrequency(100.0f);
}

UAbilitySystemComponent* ASMPlayerState::GetAbilitySystemComponent() const
{
	return SMAbilitySystemComponent;
}

void ASMPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ASMPlayerState* NewPS = Cast<ASMPlayerState>(PlayerState);
	if (IsValid(NewPS) == false) return;

	NewPS->bIsHost = bIsHost;
	NewPS->bIsReady = bIsReady;
	NewPS->SelectedWeaponIndex = SelectedWeaponIndex;
	NewPS->SelectedMaterialIndex = SelectedMaterialIndex;
	NewPS->SetPlayerName(GetPlayerName());
}

void ASMPlayerState::ResetForRespawn()
{
	if (AttributeSet)
	{
		AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
	}

	if (SMAbilitySystemComponent)
	{
		FGameplayEffectQuery EmptyQuery;
		SMAbilitySystemComponent->RemoveActiveEffects(EmptyQuery);
	}

	SM_LOG(this, LogSM, Warning, TEXT("[%s]의 [PlayerState] 체력 복구"), *GetPlayerName());
}

void ASMPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASMPlayerState, bIsHost);
	DOREPLIFETIME(ASMPlayerState, bIsReady);
	DOREPLIFETIME(ASMPlayerState, SelectedWeaponIndex);
	DOREPLIFETIME(ASMPlayerState, SelectedMaterialIndex);
}

void ASMPlayerState::SetSelectedWeaponIndex(int32 NewIndex)
{
	ensureMsgf(HasAuthority(), TEXT("SetSelectedWeaponIndex must be called on server"));
	if (HasAuthority() == false) return;
	SelectedWeaponIndex = NewIndex;
}

void ASMPlayerState::SetSelectedMaterialIndex(int32 NewIndex)
{
	ensureMsgf(HasAuthority(), TEXT("SetSelectedMaterialIndex must be called on server"));
	if (HasAuthority() == false) return;
	SelectedMaterialIndex = NewIndex;
}

void ASMPlayerState::OnRep_SelectedWeaponIndex()
{
	ASMPlayerCharacter* PlayerCharacter = Cast<ASMPlayerCharacter>(GetPawn());
	if (IsValid(PlayerCharacter) == false) return;

	PlayerCharacter->ApplyCustomization();
}

void ASMPlayerState::OnRep_SelectedMaterialIndex()
{
	ASMPlayerCharacter* PlayerCharacter = Cast<ASMPlayerCharacter>(GetPawn());
	if (IsValid(PlayerCharacter) == false) return;

	PlayerCharacter->ApplyCustomization();
}
