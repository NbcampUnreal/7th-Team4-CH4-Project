#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMBuildingHPBarWidget.generated.h"

class UProgressBar;

/**
 * 건물 위에 떠있는 HP 바 위젯
 */
UCLASS()
class SAGOMAGIC_API USMBuildingHPBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** HP 수치로 바 갱신 */
	void UpdateHPBar(float CurrentHP, float MaxHP);
	/** 최대 체력 초기 설정 */
	void SetMaxHP(float InMaxHP);

protected:
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_BuildingHP;
	
	UPROPERTY(EditAnywhere, Category = "UI Settings")
	float InterpSpeed = 10.f;

private:
	float TargetPercent  = 1.f;
	float CurrentPercent = 1.f;
	float CachedMaxHP    = 1.f;
};