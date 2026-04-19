// ASMTitlePlayerController.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SMTitlePlayerController.generated.h"

class USMTitleWidget;
class USMSessionSubsystem;
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
	void TravelToCheck(const FString& Address);

private:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USMTitleWidget> MainWidgetClass;
	UPROPERTY(VisibleAnywhere, Category = "UI")
	TObjectPtr<USMTitleWidget> MainWidgetInstance;

	UPROPERTY()
	TObjectPtr<USMSessionSubsystem> SessionSubsystem;

	void ShowMainWidget();

	//Steam 오버레이 초대 수락 시 사용
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	void TravelToServer();

	void BindSessionDelegates();
	void UnbindSessionDelegates();
};
