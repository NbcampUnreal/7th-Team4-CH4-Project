#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "SMEnemyHPBarComponent.generated.h"


UCLASS()
class SAGOMAGIC_API USMEnemyHPBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    USMEnemyHPBarComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** GAS - 데이터 받은 후 작업 */
    /*UFUNCTION()
    void OnHPChanged(const FOnAttributeChangeData& Data);*/

private:
    /** 일정 시간 후 UI 숨기는 함수 */
    void HideHPBar();
    FTimerHandle HideTimerHandle;

    UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
    float DisplayDuration = 2.0f;

    /** GAS - 데이터 받은 후 작업 */
    /*UPROPERTY()
    class UAbilitySystemComponent* ASC;*/
};
