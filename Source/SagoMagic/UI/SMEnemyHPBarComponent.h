#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "SMEnemyHPBarComponent.generated.h"


class UAbilitySystemComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMEnemyHPBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    USMEnemyHPBarComponent();
    
    /** 몬스터가 ASC 초기화를 완료한 후 호출할 함수 */
    void InitializeHPBar(UAbilitySystemComponent* InASC);

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** GAS - 체력 변경 시 호출될 콜백 함수 */
    void OnHPChanged(const FOnAttributeChangeData& Data);
    /** 일정 시간 후 UI 숨기는 함수 */
    void HideHPBar();
    
private:
    /** 캐싱해둘 ASC 포인터 */
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;
    
    FTimerHandle HideTimerHandle;

    UPROPERTY(EditAnywhere, Category = "UI")
    float DisplayDuration = 2.0f;
};
