#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/WorldSubsystem.h"
#include "SMAsyncDataManager.generated.h"

DECLARE_DELEGATE(FOnAssetLoadComplete);

/**
 * PrimaryDataAsset을 ID 기반으로 비동기 로드/캐시 관리
 * 서버 전용, L_Play에서만 생성
 */
UCLASS()
class SAGOMAGIC_API USMAsyncDataManager : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;
	
	static USMAsyncDataManager* Get(const UObject* WorldContextObject);
	
	/**
	 * 특정 ID 목록만 비동기 로드
	 * 이미 로드된 에셋은 스킵
	 * @param AssetIDs : 로드할 PrimaryAssetId 목록
	 * @param OnComplete : 전부 완료됐을 때 호출되는 콜백
	*/
	void LoadAssetsByID(const TArray<FPrimaryAssetId>& AssetIDs, FOnAssetLoadComplete OnComplete);
	
	/**
	 * 특정 타입의 에셋 언로드
	*/
	void UnloadAssetsByID(const TArray<FPrimaryAssetId>& AssetIDs);
	
	/** 로드된 에셋 반환 (없으면 nullptr) */
	UPrimaryDataAsset* GetLoadAsset(const FPrimaryAssetId& AssetID) const;
	
	/** 로드 여부 확인 */
	bool IsAssetLoaded(const FPrimaryAssetId& AssetID) const;
	
private:
	//로드된 에셋 캐시
	UPROPERTY()
	TMap<FPrimaryAssetId, TObjectPtr<UPrimaryDataAsset>> LoadedAssets;
	
	//로드 핸들 (살아있어야 로드 유지됨)
	TMap<FPrimaryAssetId, TSharedPtr<FStreamableHandle>> LoadHandles;
};

