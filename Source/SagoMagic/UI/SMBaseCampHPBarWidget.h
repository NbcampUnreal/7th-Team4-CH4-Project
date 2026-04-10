#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/SMGameplayMessages.h"
#include "SMBaseCampHPBarWidget.generated.h"

class UProgressBar;
class UTextBlock;
/**
 * 베이스 캠프 체력 프로그래스바를 띄우는 위젯
 */
UCLASS()
class SAGOMAGIC_API USMBaseCampHPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_BaseCampHP;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_BaseCampHP;
	
private:
	/** 방송을 수신했을 때 실행될 콜백 함수 */
	void OnBaseCampMessageReceived(FGameplayTag Channel, const FBaseCampMsg& Message);

	/** Listener 해제를 위한 핸들러 */
	UPROPERTY()
	FGameplayMessageListenerHandle BaseCampListenerHandle;
};
