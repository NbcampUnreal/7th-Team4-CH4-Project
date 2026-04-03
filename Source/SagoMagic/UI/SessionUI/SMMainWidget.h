// SMMainWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMMainWidget.generated.h"

class ASMPlayerController;
class USMSessionSubsystem;
class UEditableTextBox;
class UButton;
/**
 * 테스트용 메인 UI입니다.
 */
UCLASS()
class SAGOMAGIC_API USMMainWidget : public UUserWidget
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

	ASMPlayerController* GetSMPlayerController() const;
};
