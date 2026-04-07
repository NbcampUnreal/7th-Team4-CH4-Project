#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPlayerStatusWidget.generated.h"


class USMPlayerHPBarWidget;
class USMPlayerGoldWidget;
class UAbilitySystemComponent;

/**
 * 플레이어의 상태(HP, 골드)를 모아두는 컨테이너 위젯 클래스
 * HUDManager로부터 ASC 받아 하위 세부 위젯에게 데이터 분배
 */
UCLASS()
class SAGOMAGIC_API USMPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="UI")
	void InitializeStatus(UAbilitySystemComponent* InASC);
	
protected:
	/** 하위 위젯 바인딩 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USMPlayerHPBarWidget> WBP_PlayerBar;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USMPlayerGoldWidget> WBP_GoldDisplay;
};
