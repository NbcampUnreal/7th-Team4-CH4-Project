#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPlayerStatusWidget.generated.h"


class USMPlayerStatusWidget;
class UAbilitySystemComponent;

/**
 * 플레이어의 상태(HP, 골드 등)를 모아두는 컨테이너 위젯 클래스
 * WBP_PlayerStatus의 부모 클래스
 */
UCLASS()
class SAGOMAGIC_API USMPlayerStatusWidget : public UUserWidget
{
	GENERATED_BODY()
};
