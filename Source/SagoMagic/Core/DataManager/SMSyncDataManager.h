#pragma once

#include "CoreMinimal.h"
#include "Data/SMMonsterData.h"
#include "Data/SMWaveData.h"
#include "Subsystems/WorldSubsystem.h"
#include "SMSyncDataManager.generated.h"

/**
 * DataTable을 관리하는 전역 클래스입니다.
 * WorldSubsystem은 클라, 서버 모두 각각 생성됩니다.
 * 따라서 ShouldCreateSubsystem로 클라, 생성되는 레벨을 감지하여 로드합니다.
 * 
 * 사용 예시 
 * USMSyncDataManager* Manager = USMSyncDataManager::Get(GetWorld());
 * if (Manager)
 * {
 *		const FWaveRowData& WaveInfo = Manager->GetWaveData(1);//1은 WaveLevel
 * }
 */
UCLASS()
class SAGOMAGIC_API USMSyncDataManager : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static USMSyncDataManager* Get(const UObject* WorldContext);
	
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
 	
	/** Monster 데이터 조회 - EMonsterType 키 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	FSMMonsterData GetMonsterData(EMonsterType MonsterType) const;
	
	/** Wave 데이터 조회 - WaveLevel 키 */
	UFUNCTION(BlueprintCallable, Category = "Data")
	FSMWaveData GetWaveData(int32 WaveLevel) const;

public:
 	template<typename RowType, typename KeyType>
 	static void LoadAndCacheTable(const TCHAR* Path, TMap<KeyType, RowType>& OutCache,
 		TFunctionRef<KeyType(const RowType*)> KeySelector);
 	
private:
	/** DataTable 관리 */
 	UPROPERTY()
 	TMap<int32, FSMWaveData> WaveCache;
 	
	UPROPERTY()
	TMap<EMonsterType, FSMMonsterData> MonsterCache;
	
	/** Monster DT 실제 경로 */
	FString MonsterDataTablePath = TEXT("/Game/SagoMagic/Data/DataTables/MonsterData/DT_Monster.DT_Monster");
	/** Wave DT 실제 경로 */
	FString WaveDataTablePath = TEXT("/Game/SagoMagic/Data/DataTables/WaveData/DT_Wave.DT_Wave");
};

template <typename RowType, typename KeyType>
void USMSyncDataManager::LoadAndCacheTable(const TCHAR* Path, TMap<KeyType, RowType>& OutCache,
	TFunctionRef<KeyType(const RowType*)> KeySelector)
{
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr,Path);
	if (!DataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[SyncDataManager] Failed to load DataTable at : %s"),Path);
		return;
	}
	
	FString ContextString;
	TArray<RowType*> AllRows;
	DataTable->GetAllRows(ContextString, AllRows);
	for (const RowType* Row : AllRows)
	{
		if (Row)
		{
			OutCache.Add(KeySelector(Row), *Row);
		}
	}
}
