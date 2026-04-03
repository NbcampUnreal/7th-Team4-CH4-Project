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
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	
	void UpdateHPText(float CurrentHP, float MaxHP);
	
	/** GAS - 체력 변경 시 호출될 콜백 함수 */
	void OnHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	
protected:
	FText HPTextTemplate;
	
	UPROPERTY(meta=(BindWidget))
	UProgressBar* ProgressBar_PlayerHP;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TextBlock_PlayerHP;
	
	float TargetPercent = 1.0f;
	float CurrentPercent = 1.0f;
	
	float CachedMaxHP = 1.0f;
	float CachedCurrentHP = 1.0f;
	
	UPROPERTY(EditAnywhere, Category = "UI Settings")
	float InterpSpeed = 15.0f;
	
private:
	/** 메모리 누수 및 크래시 방지 */
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	
	UAbilitySystemComponent* GetPlayerASC() const;
};
