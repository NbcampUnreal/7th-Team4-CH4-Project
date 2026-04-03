#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "SMEnemyHPBarComponent.generated.h"


class UAbilitySystemComponent;

UCLASS()
class SAGOMAGIC_API USMEnemyHPBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    USMEnemyHPBarComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** GAS - 체력 변경 시 호출될 콜백 함수 */
    void OnHPChanged(const FOnAttributeChangeData& Data);

private:
    /** 일정 시간 후 UI 숨기는 함수 */
    void HideHPBar();
    FTimerHandle HideTimerHandle;

    UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
    float DisplayDuration = 2.0f;

    /** 캐싱해둘 ASC 포인터 */
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;
};
