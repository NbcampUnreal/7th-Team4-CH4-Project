#include "SMSyncDataManager.h"

#include "GameplayTags/Character/SMSkillTag.h"

bool USMSyncDataManager::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (!World) return false;
	if (World->GetNetMode() == NM_Client) return false;//클라이언트 차단
	return World->GetMapName().Contains(TEXT("L_Play"));//L_Play에서만 생성
}

void USMSyncDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	LoadAndCacheTable<FSMMonsterData, EMonsterType>(
	*MonsterDataTablePath,
	MonsterCache,
	[](const FSMMonsterData* Row) {return Row->MonsterType;}
		);
	
	LoadAndCacheTable<FSMWaveData, int32>(
	*WaveDataTablePath,
	WaveCache,
	[](const FSMWaveData* Row) {return Row->WaveLevel;}
	);
	
	LoadAndCacheTable<FSMSkillData, FGameplayTag>(
		*SkillDataTablePath,
		SkillCache,
		[](const FSMSkillData* Row){return Row->SkillTag;}
		);
	
}

USMSyncDataManager* USMSyncDataManager::Get(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!World) return nullptr;
	return World->GetSubsystem<USMSyncDataManager>(); // 클라이언트는 nullptr 반환
}

FSMMonsterData USMSyncDataManager::GetMonsterData(EMonsterType MonsterType) const
{
	const FSMMonsterData* Found = MonsterCache.Find(MonsterType);
	if (!Found)
	{
		UE_LOG(LogTemp, Error, TEXT("[SMSyncDataManager] MonsterType %d 없음"),(int32)MonsterType);
		return FSMMonsterData();
	}
	return *Found;
}

FSMWaveData USMSyncDataManager::GetWaveData(int32 WaveLevel) const
{
	const FSMWaveData* Found = WaveCache.Find(WaveLevel);
	if (!Found)
	{
		UE_LOG(LogTemp, Error, TEXT("[SMSyncDataManager] WaveLevel %d 없음"),(int32)WaveLevel);
		return FSMWaveData();
	}
	return *Found;
}

FSMSkillData USMSyncDataManager::GetSkillData(FGameplayTag SkillTag) const
{
	const FSMSkillData* Found = SkillCache.Find(SkillTag);
	if (!Found)
	{
		UE_LOG(LogTemp, Error, TEXT("[SMSyncDataManager] SkillTag %s 없음"),*SkillTag.ToString());
		return FSMSkillData();
	}
	return *Found;
}
