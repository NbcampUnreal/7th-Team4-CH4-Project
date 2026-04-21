//USMPlayerNicknameWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMPlayerNicknameWidget.generated.h"

/**
 * 플레이어 닉네임 위젯입니다.
 */
UCLASS()
class SAGOMAGIC_API USMPlayerNicknameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetPlayerNickName(const FString& InNickName);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> NicknameTextBlock;
	
};
