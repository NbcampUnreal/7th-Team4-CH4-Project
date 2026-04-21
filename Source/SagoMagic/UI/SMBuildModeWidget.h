#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "SMBuildModeWidget.generated.h"

class UHorizontalBox;
class UBorder;

/**
 * 건축 모드 UI 위젯
 */
UCLASS()
class SAGOMAGIC_API USMBuildModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> SlotContainer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Color")
	FLinearColor SelectedColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Color")
	FLinearColor DefaultColor = FLinearColor(0.f, 0.f, 0.f, 0.8f);

private:
	void OnBuildModeMessage(FGameplayTag Channel, const FBuildModeMsg& Message);
	
	void UpdateSlotHighlight(int32 SelectedIndex);
	
	FGameplayMessageListenerHandle BuildModeListenerHandle;
	
	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SlotBorders;
};