#include "GA_BuildPlace.h"

#include "AbilitySystemComponent.h"
#include "SagoMagic.h"
#include "Building/SMFenceBuilding.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/Character/SMCharacterTag.h"
#include "GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "Kismet/GameplayStatics.h"

UGA_BuildPlace::UGA_BuildPlace()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = SMCharacterTag::Ability_Build_Place;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
}
ASMGridManager* UGA_BuildPlace::GetGridManager()
{
	if (!CachedGridManager)
	{
		CachedGridManager = Cast<ASMGridManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ASMGridManager::StaticClass()));
	}
	return CachedGridManager;
}
void UGA_BuildPlace::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const FSMBuildPlaceTargetData* PlaceData = 
		static_cast<const FSMBuildPlaceTargetData*>(
			TriggerEventData->TargetData.Get(0));
	if (!PlaceData || PlaceData->CellInfos.IsEmpty())
	{
		SM_LOG(this,LogSM,Error,TEXT("[GA_BuildPlace] TargetData 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	ASMGridManager* GridManager = GetGridManager();
	if (!GridManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	USMSyncDataManager* DM = GetWorld()->GetSubsystem<USMSyncDataManager>();
	if (!GridManager || !DM)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const FSMBuildingData* BuildingData = DM->GetBuildData(PlaceData->BuildingType);
	if (!BuildingData || !BuildingData->BuildingClass)
	{
		SM_LOG(this, LogSM, Error, TEXT("[GA_BuildPlace] BuildingData 없음"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!ServerValidateCells(GridManager, PlaceData->CellInfos))
	{
		SM_LOG(this, LogSM, Error, TEXT("[GA_BuildPlace] 이미 점유된 셀 존재"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (!ApplyBuildCost(Handle, ActorInfo, ActivationInfo, PlaceData->BuildingType, BuildingData->Cost * PlaceData->CellInfos.Num()))
	{
		SM_LOG(this, LogSM, Error, TEXT("[GA_BuildPlace] 골드 부족"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	int32 OwnerId = 0;
	if (APlayerState* PS = ActorInfo->PlayerController->GetPlayerState<APlayerState>())
	{
		OwnerId = PS->GetPlayerId();
	}
	if (!SpawnAndRegister(GridManager, PlaceData->CellInfos, BuildingData->BuildingClass, PlaceData->BuildingType, OwnerId, BuildingData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_BuildPlace::ServerValidateCells(ASMGridManager* GridManager, const TArray<FSMCellPlaceInfo>& CellInfos)
{
	for (const FSMCellPlaceInfo& Info : CellInfos)
	{
		if (!GridManager->IsValidGridPosition(Info.Grid.X, Info.Grid.Y))
		{
			return false;
		}
		if (!GridManager->IsCellEmpty(Info.Grid.X, Info.Grid.Y))
			return false;
	}
	return true;
}

bool UGA_BuildPlace::ApplyBuildCost(const FGameplayAbilitySpecHandle& Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo& ActivationInfo,
	EGridBuildingType BuildingType, int32 CostAmount)
{
	TSubclassOf<UGameplayEffect>* CostClass = BuildCostEffects.Find(BuildingType);
	if (!CostClass || !(*CostClass)) return true;
	
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return false;
	float CurrentGold = ASC->GetNumericAttribute(USMPlayerAttributeSet::GetGoldAttribute());
	if (CurrentGold < (float)CostAmount) return false;
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(*CostClass, 1.f, Context);
	if (!Spec.IsValid()) return false;
	
	Spec.Data->SetSetByCallerMagnitude(SMCharacterTag::Build_GoldCost, -(float)CostAmount);
	
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	return true;
}

bool UGA_BuildPlace::SpawnAndRegister(ASMGridManager* GridManager, const TArray<FSMCellPlaceInfo>& CellInfos,
	TSubclassOf<AActor> BuildingClass,EGridBuildingType BuildingType, int32 OwnerId, const FSMBuildingData* BuildingData)
{
	TArray<ASMBaseBuilding*> SpawnedActors;
	
	for (const FSMCellPlaceInfo& Info : CellInfos)
	{
		FVector SpawnPos = GridManager->GridToWorldWithHeight(Info.Grid.X, Info.Grid.Y);
		SpawnPos.Z +=2.f;
		FRotator SpawnRot = FRotator(0.f, Info.Yaw, 0.f);
		
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = 
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			BuildingClass, SpawnPos, SpawnRot, Params);
		ASMBaseBuilding* Building = Cast<ASMBaseBuilding>(SpawnedActor);
		
		if (!Building)
		{
			Rollback(GridManager, SpawnedActors);
			return false;
		}
		
		int32 RotationIndex = FMath::RoundToInt(Info.Yaw / 90.f) % 4;
		bool bPlaced = GridManager->PlaceBuilding(
			Info.Grid.X, Info.Grid.Y,
			FIntPoint(1,1),
			RotationIndex,
			BuildingType,
			Building,
			OwnerId);

		if (!bPlaced)
		{
			Building->Destroy();
			Rollback(GridManager, SpawnedActors);
			return false;
		}
		if (Info.bIsCorner)
		{
			if (ASMFenceBuilding* Fence = Cast<ASMFenceBuilding>(Building))
			{
				Fence->ConvertToCorner(Info.Yaw);
			}
		}
		Building->InitBuilding(Info.Grid, BuildingData->bIsDestructible, BuildingData->MaxHealth);
		SpawnedActors.Add(Building);
	}	
	return true;
}

void UGA_BuildPlace::Rollback(ASMGridManager* GridManager, TArray<ASMBaseBuilding*>& SpawnedActors)
{
	for (ASMBaseBuilding* Actor : SpawnedActors)
	{
		if (!Actor) continue;
		GridManager->ClearCellsByActor(Actor);
		Actor->Destroy();
	}
	SpawnedActors.Empty();
}
