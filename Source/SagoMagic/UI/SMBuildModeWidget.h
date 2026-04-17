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
	
	UPROPERTY(EditDefaultsOnly, Category="BuildMode|Style")
	FLinearColor SelectedColor = FLinearColor::White;
	UPROPERTY(EditDefaultsOnly, Category="BuildMode|Style")
	FLinearColor DefaultColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

private:
	void OnBuildModeMessage(FGameplayTag Channel, const FBuildModeMsg& Message);
	/** 색상을 칠해주는 핵심 함수 */
	void UpdateSlotHighlight(int32 SelectedIndex);

	FGameplayMessageListenerHandle BuildModeListenerHandle;
	
	UPROPERTY()
	TArray<TObjectPtr<UBorder>> SlotBorders;
};