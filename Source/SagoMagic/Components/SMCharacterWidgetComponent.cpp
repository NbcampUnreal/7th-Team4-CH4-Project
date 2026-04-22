#include "SMCharacterWidgetComponent.h"
#include "AbilitySystemComponent.h"
#include "Core/DataManager/SMSoundManager.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "UI/SMGoldFloatingText.h"

USMCharacterWidgetComponent::USMCharacterWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USMCharacterWidgetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedASC)
		CachedASC->GetGameplayAttributeValueChangeDelegate(
			USMPlayerAttributeSet::GetGoldAttribute()).Remove(GoldChangeHandle);
		
	Super::EndPlay(EndPlayReason);
}

void USMCharacterWidgetComponent::InitializeWithASC(UAbilitySystemComponent* InASC)
{
	if (!InASC) return;
	
	if (CachedASC)
		CachedASC->GetGameplayAttributeValueChangeDelegate(
			USMPlayerAttributeSet::GetGoldAttribute()).Remove(GoldChangeHandle);
	
	CachedASC = InASC;
	GoldChangeHandle = CachedASC->GetGameplayAttributeValueChangeDelegate(
		USMPlayerAttributeSet::GetGoldAttribute()).AddUObject(this, &USMCharacterWidgetComponent::OnGoldChanged);
}

void USMCharacterWidgetComponent::OnGoldChanged(const FOnAttributeChangeData& Data)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled()) return;
	
	float Delta = Data.NewValue - Data.OldValue;
	if (FMath::Abs(Delta) < 0.01f) return;
	
	SpawnGoldFloatingText(Delta);
	
	if (USMSoundManager* SM = USMSoundManager::Get(this))
	{
		SM->PlaySoundUI(TEXT("Coin"));
	}
}

void USMCharacterWidgetComponent::SpawnGoldFloatingText(float GoldDelta)
{
	if (!GoldTextClass) return;
	
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer) return;
	
	AActor* Owner = GetOwner();
	if (!Owner) return;
	
	FVector SpawnLocation = Owner->GetActorLocation() + FVector(0.f, 0.f, 120.f);
	
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ASMGoldFloatingText* GoldText = World->SpawnActor<ASMGoldFloatingText>(
		GoldTextClass, SpawnLocation, FRotator::ZeroRotator, Params);
	
	if (GoldText)
		GoldText->SetGoldValue(GoldDelta);
	
}
