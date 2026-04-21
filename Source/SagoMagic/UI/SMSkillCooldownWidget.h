#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "SMSkillCooldownWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UAbilitySystemComponent;

UCLASS()
class SAGOMAGIC_API USMSkillCooldownWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void InitializeWithASC(UAbilitySystemComponent* InASC, FGameplayTag InCooldownTag);

    bool IsOnCooldown() const;
    
protected:
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ProgressBar_Cooldown;
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> TextBlock_Cooldown;

private:
    void UnbindASC();

    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> BoundASC;

    UFUNCTION()
    void OnCooldownTagChanged(const FGameplayTag Tag, int32 NewCount);
    
    FGameplayTag CooldownTag;

    float TotalCooldown = 0.f;
    bool bOnCooldown = false;
};