// SMTitleWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMTitleWidget.generated.h"

class UTextBlock;
class ASMTitlePlayerController;
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
	
	void ShowLobbyFullMessage();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

private:
	
	UFUNCTION()
	void OnHostButtonClicked();

	void HideLobbyFullText();

	ASMTitlePlayerController* GetSMTitlePlayerController() const;
	
	//위젯 컴포넌트 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> IPInputBox;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LobbyFullText;
	
	FTimerHandle LobbyFullTextTimer;
};
