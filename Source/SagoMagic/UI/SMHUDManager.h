#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMHUDManager.generated.h"


class USMPlayerHPBarWidget;
class UAbilitySystemComponent;

/**
 * 인게임 HUD 루트 위젯 - 뷰포트에 올라가며 자식 위젯들 소유
 * 폰 빙의 완료 후 InitializeHUD()를 호출해 ASC 연결
 */
UCLASS()
class SAGOMAGIC_API USMHUDManager : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** ASC를 받아 각 자식 위젯에 전달 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeHUD(UAbilitySystemComponent* InPlayerASC);
	/** 리스폰 등으로 새 폰이 생겼을 때 재연결 */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void RefreshHUD(UAbilitySystemComponent* InPlayerASC);
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidgetOptional), BlueprintReadOnly, Category="HUD")
	TObjectPtr<USMPlayerHPBarWidget> WBP_PlayerBar;
};
