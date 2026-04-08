#include "SMAsyncDataManager.h"
#include "Engine/AssetManager.h"
bool USMAsyncDataManager::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (!World) return false;
	return World->GetMapName().Contains(TEXT("L_Play"));
}

void USMAsyncDataManager::Deinitialize()
{
	//핸들 전부 해제
	for (auto& Pair : LoadHandles)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->ReleaseHandle();
		}
	}
	LoadHandles.Empty();
	LoadedAssets.Empty();
	
	Super::Deinitialize();
}

USMAsyncDataManager* USMAsyncDataManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;
	return World->GetSubsystem<USMAsyncDataManager>();
}

void USMAsyncDataManager::LoadAssetsByID(const TArray<FPrimaryAssetId>& AssetIDs, FOnAssetLoadComplete OnComplete)
{
	//이미 로드된 에셋 제외
	TArray<FPrimaryAssetId> IDsToLoad;
	for (const FPrimaryAssetId& ID : AssetIDs)
	{
		if (ID.IsValid() && !LoadedAssets.Contains(ID))
		{
			IDsToLoad.Add(ID);
		}
	}
	//전부 이미 로드됐으면 즉시 콜백
	if (IDsToLoad.IsEmpty())
	{
		if (OnComplete.IsBound())
		{
			OnComplete.Execute();
		}
		return;
	}
	
	UE_LOG(LogTemp,Log,TEXT("[AsyncDataManager] %d개 에셋 로드 시작"),IDsToLoad.Num());
	
	UAssetManager& Manager = UAssetManager::Get();
	TSharedPtr<FStreamableHandle> Handle = Manager.LoadPrimaryAssets(
		IDsToLoad,
		TArray<FName>(),  // 번들 없이 전체 로드
		FStreamableDelegate::CreateLambda([this, IDsToLoad, OnComplete]()
		{
			// 로드 완료 → 캐시 저장
			UAssetManager& Mgr = UAssetManager::Get();
			for (const FPrimaryAssetId& ID : IDsToLoad)
			{
				if (UPrimaryDataAsset* Asset = Cast<UPrimaryDataAsset>(
					Mgr.GetPrimaryAssetObject(ID)))
				{
					LoadedAssets.Add(ID, Asset);
					UE_LOG(LogTemp, Log, TEXT("[AsyncDataManager] 로드 완료: %s"), *ID.ToString());
				}
			}

			if (OnComplete.IsBound())
			{
				OnComplete.Execute();
			}
		})
	);
	
	//핸들 저장 (해제되면 언로드됨)
	if (Handle.IsValid())
	{
		for (const FPrimaryAssetId& ID : IDsToLoad)
		{
			LoadHandles.Add(ID,Handle);
		}
	}
	
}

void USMAsyncDataManager::LoadAssetsByIDWithBundles(const TArray<FPrimaryAssetId>& AssetIDs, const TArray<FName>& Bundles, FOnAssetLoadComplete OnComplete)
{
	TArray<FPrimaryAssetId> IDsToLoad;
	for (const FPrimaryAssetId& ID : AssetIDs)
	{
		if (ID.IsValid() && !LoadedAssets.Contains(ID))
		{
			IDsToLoad.Add(ID);
		}
	}

	if (IDsToLoad.IsEmpty())
	{
		if (OnComplete.IsBound())
		{
			OnComplete.Execute();
		}
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[AsyncDataManager] %d개 에셋 번들 로드 시작"), IDsToLoad.Num());

	UAssetManager& Manager = UAssetManager::Get();
	TSharedPtr<FStreamableHandle> Handle = Manager.LoadPrimaryAssets(
		IDsToLoad,
		Bundles,
		FStreamableDelegate::CreateLambda([this, IDsToLoad, OnComplete]()
		{
			UAssetManager& Mgr = UAssetManager::Get();
			for (const FPrimaryAssetId& ID : IDsToLoad)
			{
				if (UPrimaryDataAsset* Asset = Cast<UPrimaryDataAsset>(Mgr.GetPrimaryAssetObject(ID)))
				{
					LoadedAssets.Add(ID, Asset);
					UE_LOG(LogTemp, Log, TEXT("[AsyncDataManager] 번들 로드 완료: %s"), *ID.ToString());
				}
			}

			if (OnComplete.IsBound())
			{
				OnComplete.Execute();
			}
		})
	);

	if (Handle.IsValid())
	{
		for (const FPrimaryAssetId& ID : IDsToLoad)
		{
			LoadHandles.Add(ID, Handle);
		}
	}
}

void USMAsyncDataManager::UnloadAssetsByID(const TArray<FPrimaryAssetId>& AssetIDs)
{
	for (const FPrimaryAssetId& ID : AssetIDs)
	{
		if (TSharedPtr<FStreamableHandle>* Handle = LoadHandles.Find(ID))
		{
			if (Handle->IsValid())
			{
				(*Handle)->ReleaseHandle();
			}
			LoadHandles.Remove(ID);
		}
		LoadedAssets.Remove(ID);
	}
}

UPrimaryDataAsset* USMAsyncDataManager::GetLoadAsset(const FPrimaryAssetId& AssetID) const
{
	if (const TObjectPtr<UPrimaryDataAsset>* Found = LoadedAssets.Find(AssetID))
	{
		return Found->Get();
	}
	return nullptr;
}

bool USMAsyncDataManager::IsAssetLoaded(const FPrimaryAssetId& AssetID) const
{
	return LoadedAssets.Contains(AssetID);
}
