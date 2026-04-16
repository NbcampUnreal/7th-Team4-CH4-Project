#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayEffectTypes.h"
#include "SMEnemyHPBarComponent.generated.h"


class UAbilitySystemComponent;
class ASMDamageFloatingText;

UCLASS(meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMEnemyHPBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    USMEnemyHPBarComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** 몬스터가 ASC 초기화를 완료한 후 호출할 함수 */
    void InitializeHPBar(UAbilitySystemComponent* InASC);
    /** GAS - 체력 변경 시 호출될 콜백 함수 */
    void OnHPChanged(const FOnAttributeChangeData& Data);
    /** 일정 시간 후 UI 숨기는 함수 */
    void HideHPBar();
    
private:
    /** ASC를 안전하게 가져오기 위해 재시도 */
    void TryInitASC();
    /** 데미지 플로팅 텍스트 띄우기 위한 함수 */
    void SpawnDamageFloatingText(float DamageAmount);
    
    /** 캐싱해둘 ASC 포인터 */
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;
    
    FTimerHandle HideTimerHandle;
    FTimerHandle ASC_InitTimerHandle;
    
    UPROPERTY(EditAnywhere, Category = "UI")
    float DisplayDuration = 2.0f;
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<ASMDamageFloatingText> DamageTextClass;
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    float DamageTextSpawnCooldown = 0.1f;

    /** 마지막으로 텍스트를 스폰한 시간 기록용 */
    float LastDamageTextSpawnTime = 0.f;
};
