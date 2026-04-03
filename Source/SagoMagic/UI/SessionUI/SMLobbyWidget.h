// SMLobbyWidget.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/SessionSubsystem/SMPlayerSlotInfo.h"
#include "SMLobbyWidget.generated.h"

class UButton;
class UVerticalBox;
class ASMPlayerController;
class ASMLobbyGameState;
/**
 * 테스트용 로비 UI입니다.
 */
UCLASS()
class SAGOMAGIC_API USMLobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void LobbySetup();

protected:
    virtual bool Initialize() override;
    virtual void NativeDestruct() override;

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ReadyButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> StartButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> InviteButton;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UVerticalBox> PlayerSlotBox;

    UFUNCTION()
    void OnReadyButtonClicked();

    UFUNCTION()
    void OnStartButtonClicked();

    UFUNCTION()
    void OnInviteButtonClicked();

    // GameState 슬롯 변경 시 호출
    UFUNCTION()
    void OnPlayerSlotsUpdated();

    void UpdateSlotUI(const TArray<FSMPlayerSlotInfo>& Slots);
    void UpdateButtonVisibility();

    void TearDown();

    ASMPlayerController* GetSMPlayerController() const;
    ASMLobbyGameState*   GetLobbyGameState() const;
};
