#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "SMPlayerGoldWidget.generated.h"


class UTextBlock;
class UAbilitySystemComponent;
/**
 * 전달받은 ASC 구독, Gold Attribute 감지하여 현재 골드 표시
 */
UCLASS()
class SAGOMAGIC_API USMPlayerGoldWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** HUDManager에서 위젯 생성 -> 대상 캐릭터의 ASC를 전달하여 위젯 초기화 */
	UFUNCTION(BlueprintCallable)
	void InitializeWithASC(UAbilitySystemComponent* InASC);
	
protected:
	virtual void NativeDestruct() override;
	
private:
	/** GAS - 골드 수치 변경될 때 호출 */
	void OnGoldChanged(const FOnAttributeChangeData& Data);
	void UpdateGoldText(float CurrentGold);
	/** 기존 바인딩된 ASC 연결 해제 */
	void UnbindASC();

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Gold;

private:
	/** 현재 위젯이 관찰하고 있는 ASC */
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	/** GAS 델리게이트 등록 해제를 위한 핸들 - 메모리 누수 및 크래시 방지 */
	FDelegateHandle GoldChangedHandle;
};
