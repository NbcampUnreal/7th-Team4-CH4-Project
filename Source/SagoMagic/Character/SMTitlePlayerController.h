// ASMTitlePlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMTitlePlayerController.generated.h"

class USMTitleWidget;
/**
 * 타이틀전용 플레이어 컨트롤러
 * L_Title에서 IP입력을 받아서 dedicate server에 연결하는 역할을 합니다.
 */
UCLASS()
class SAGOMAGIC_API ASMTitlePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	
public:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// USMainWidget에서 호출
	void SetPendingServerAddress(const FString& Address);
	
private:
	UPROPERTY(EditDefaultsOnly,Category = "UI")
	TSubclassOf<USMTitleWidget> MainWidgetClass;
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<USMTitleWidget> MainWidgetInstance;
	
	FString PendingServerAddress = TEXT("127.0.0.1:17777");
	
	UPROPERTY(EditDefaultsOnly,Category = "Server|Maps")
	FString MainMapName = TEXT("L_Title");
	
	void ShowMainWidget();
};
