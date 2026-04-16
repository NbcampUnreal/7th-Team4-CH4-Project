#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "SMInteractionWorldWidgetComponent.generated.h"

class USMInteractionWorldInfoWidget;
struct FSMInteractionWorldInfoData;


/**
 * 월드 상호작용 정보 위젯 컴포넌트 정의 파일
 *
 * 포함 내용:
 * - 월드 상호작용 위젯 표시/숨김
 * - UserWidget 안전 캐스팅
 * - 기본 WidgetComponent 설정
 *
 * 역할:
 * - 상호작용 대상 액터에 부착되어 월드 정보 패널 표시를 담당
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMInteractionWorldWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	USMInteractionWorldWidgetComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 현재 상호작용 월드 정보 위젯 Getter */
	UFUNCTION(BlueprintCallable, Category="Interaction World Widget Component")
	USMInteractionWorldInfoWidget* GetInteractionWorldInfoWidget() const;

	/** 상호작용 정보 표시 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction World Widget Component")
	void ShowInteractionInfo(const FSMInteractionWorldInfoData& InDisplayData);

	/** 상호작용 정보 숨김 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction World Widget Component")
	void HideInteractionInfo();
};
