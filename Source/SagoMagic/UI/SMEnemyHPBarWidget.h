#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "SMEnemyHPBarWidget.generated.h"


UCLASS()
class SAGOMAGIC_API USMEnemyHPBarWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void UpdateHPBar(float Percent);
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;

protected:
    UPROPERTY(meta=(BindWidget))
    UProgressBar* EnemyHPBar;

    float TargetPercent = 1.0f;
    float CurrentPercent = 1.0f;

    UPROPERTY(EditAnywhere, Category = "UI Settings")
    float InterpSpeed = 10.0f;
};
