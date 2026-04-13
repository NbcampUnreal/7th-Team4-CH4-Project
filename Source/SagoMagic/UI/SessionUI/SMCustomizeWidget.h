// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SMCustomizeWidget.generated.h"

class ASMPlayerCharacter;
class ASMPlayerController;
class UTextBlock;
class UButton;
/**
 * 로비 캐릭터 커스터마이징 위젯
 * 무기 (3종 개별 버튼) / 머티리얼 (좌 우 순환) / 스킬 4종 선택 후 Apply/Cancel로 확정
 */
UCLASS()
class SAGOMAGIC_API USMCustomizeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 위젯이 열릴 때 호출, PlayerState 현재 닶을 이전 값으로 저장하고 카메라/이동 전환 */
	void CustomizeSetup();
	
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;
	
	//================================
	// 무기 선택
	//================================
private:
	/** 무기 0번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponButton0;
	
	/** 무기 1번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponButton1;
	
	/** 무기 2번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> WeaponButton2;
	
	UFUNCTION()
	void OnWeaponButton0Clicked();
	UFUNCTION()
	void OnWeaponButton1Clicked();
	UFUNCTION()
	void OnWeaponButton2Clicked();
	
	void SelectWeapon(int32 WeaponIndex);
	
	//================================
	// 스킬 선택
	//================================
private:
	/** 스킬 0번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillButton0;
	
	/** 스킬 1번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillButton1;
	
	/** 스킬 2번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillButton2;
	
	/** 스킬 3번 선택 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillButton3;
	
	UFUNCTION()
	void OnSkillButton0Clicked();
	UFUNCTION()
	void OnSkillButton1Clicked();
	UFUNCTION()
	void OnSkillButton2Clicked();
	UFUNCTION()
	void OnSkillButton3Clicked();
	
	//================================
	// 머티리얼 선택
	//================================
private:
	/** 머티리얼 이전 버튼 (왼쪽)*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MaterialPrevButton;
	
	/** 머티리얼 다음 버튼 (오른쪽)*/
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MaterialNextButton;
	
	/** 현재 머티리얼 번호 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentMaterialNumber;
	
	UFUNCTION()
	void OnMaterialPrevClicked();
	UFUNCTION()
	void OnMaterialNextClicked();

	//================================
	// 확정 / 취소
	//================================
private:
	/** 적용 버튼 - 이 시점에만 ServerRPC 전송 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> ApplyButton;

	/** 취소 버튼 - 이전 값으로 로컬 복원 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;

	UFUNCTION()
	void OnApplyClicked();
	UFUNCTION()
	void OnCancelClicked();

	void CloseWidget();

	//================================
	// 내부 상태
	//================================
private:
	/** 위젯 열릴 때 저장한 이전 선택값 - 취소 시 복원용 */
	int32 PreviousWeaponIdx  = 0;
	int32 PreviousMatIdx     = 0;

	/** 현재 미리보기 중인 선택값 */
	int32 CurrentWeaponIdx   = 0;
	int32 CurrentMatIdx      = 0;

	ASMPlayerController* GetSMPlayerController() const;
	ASMPlayerCharacter*  GetSMPlayerCharacter()  const;
	
};
