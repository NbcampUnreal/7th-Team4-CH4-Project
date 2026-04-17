#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Building/FSMAStarNode.h"
#include "Building/SMBuildPlaceTargetData.h"
#include "Building/SMGridManager.h"
#include "Components/ActorComponent.h"
#include "Data/SMBuildingData.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "GameplayTags/UI/SMUITag.h"
#include "BuildingModeComponent.generated.h"

class ASMBaseBuilding;
class ASMFenceBuilding;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMBuildingModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USMBuildingModeComponent();
	
	void EnableBuildMode();
	void DisableBuildMode();
	void SetupInputBindings();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
								   FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	void OnPlaceBuilding(const FInputActionValue& Value);
	void OnRotateBuilding(const FInputActionValue& Value);
	void OnCycleBuilding(const FInputActionValue& Value);
	
	void UpdateGhostTransform();
	void UpdateGhostMaterials();
	void ClearGhostActors();
	
	bool CheckPlacementValidity(const FVector& Location, const FSMBuildingData& Data);
	
	void ConvertGhostToCorner(ASMBaseBuilding* Ghost, float Yaw);
	
	FSMCornerInfo GetCornerInfo(const TArray<FIntPoint>& Path, int32 i) const;

	/** 두 연결 방향으로 코너 Yaw 결정 */
	float GetCornerYawFromConnections(FIntPoint DirA, FIntPoint DirB) const;
	/** 경로 내부 코너 + 접합부 코너를 한 번에 계산 */
	FSMCornerInfo GetEffectiveCornerInfo(const TArray<FIntPoint>& Path, int32 i);
	
	/** 두번 째 클릭 시 서버 GA로 배치 요청 */
	void SendPlaceEvent(const TArray<FIntPoint>& Path, FIntPoint EndGrid);
	
	/** DataTable 클라 로드 */
	void LoadBuildingDataTable();
	const FSMBuildingData* GetCurrentBuildingData() const;

	UFUNCTION(Server, Reliable)
	void ServerRPC_RequestPlaceBuilding(const TArray<FSMCellPlaceInfo>& CellInfos, EGridBuildingType BuildingType);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BuildIMC;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PlaceAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> RotateAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CycleAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	float MaxBuildDistance = 1500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Building|Visuals")
	TObjectPtr<UMaterialInterface> ValidMaterial;
	
	UPROPERTY(EditDefaultsOnly, Category = "Building|Visuals")
	TObjectPtr<UMaterialInterface> InValidMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FGameplayTag BuildPlaceEventTag;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 CurrentSlotIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	int32 CurrentRotationIndex = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsBuildMode = false;
	
private:
	UPROPERTY()
	bool bIsWaitingForEndPoint = false;
	
	FIntPoint FenceStartGrid;
	
	/** 마지막으로 호버한 그리드 (불필요한 재갱신 방지) */
	FIntPoint LastHoverGrid = FIntPoint(-1, -1);
	
	/** 고스트 배열 - 첫 클릭전 : 1개, 철 클릭 후 : 경로 길이만큼 */
	UPROPERTY()
	TArray<TObjectPtr<ASMBaseBuilding>> GhostActors;
	
	UPROPERTY()
	TObjectPtr<ASMGridManager> GridManager = nullptr;
	
	bool bIsPlacementValid = false;
	
	UPROPERTY()
	TArray<FSMBuildingData> CachedBuildingData;
	
	FString BuildingDataTablePath = 
		TEXT("/Game/SagoMagic/Data/DataTables/BuildingData/DT_Building.DT_Building");
	
	bool bInputBound = false;
};
