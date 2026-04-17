#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "SMEditModeWidget.generated.h"

/**
 * 편집 모드 UI 위젯
 */
UCLASS()
class SAGOMAGIC_API USMEditModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/** 모드 활성, 비활성 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="EditMode")
	void BP_OnModeActiveChanged(bool bActive);

private:
	void OnEditModeMessage(FGameplayTag Channel, const FEditModeMsg& Message);

	FGameplayMessageListenerHandle EditModeListenerHandle;
};