#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "SMPlayerHPBarWidget.generated.h"


class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;

UCLASS()
class SAGOMAGIC_API USMPlayerHPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** HUDManager에서 위젯 생성 -> 대상 캐릭터의 ASC를 전달하여 위젯 초기화 */
	UFUNCTION(BlueprintCallable)
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	
private:
	/** 텍스트 및 UI 갱신 함수 */
	void UpdateHPBar(float CurrentHP, float MaxHP);
	/** GAS - 체력 변경 시 호출될 콜백 함수 */
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	/** 기존 바인딩된 ASC 연결 해제 */
	void UnbindASC();
	
protected:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ProgressBar_PlayerHP;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TextBlock_PlayerHP;
	UPROPERTY(EditAnywhere, Category = "UI Settings")
	float InterpSpeed = 15.0f;
	
private:
	/** 현재 위젯이 관찰하고 있는 ASC */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;
	
	float TargetPercent = 1.0f;
	float CurrentPercent = 1.0f;
	float CachedMaxHP = 1.0f;
	float CachedCurrentHP = 1.0f;
	
	FText HPTextTemplate;
	
	/** GAS 델리게이트 등록 해제를 위한 핸들 - 메모리 누수 및 크래시 방지 */
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
};
