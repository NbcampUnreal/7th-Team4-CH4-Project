// SMTitleWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMTitleWidget.generated.h"

class ASMTitlePlayerController;
class USMSessionSubsystem;
class UEditableTextBox;
class UButton;
/**
 * 테스트용 타이틀 UI입니다.
 */
UCLASS()
class SAGOMAGIC_API USMTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void MenuSetup();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

private:
	//위젯 컴포넌트 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> IPInputBox;

	UPROPERTY()
	TObjectPtr<USMSessionSubsystem> SessionSubsystem;

	UFUNCTION()
	void OnHostButtonClicked();

	UFUNCTION()
	void OnCreateSessionComplete(bool bWasSuccessful);

	void TearDown();

	ASMTitlePlayerController* GetSMTitlePlayerController() const;
};
