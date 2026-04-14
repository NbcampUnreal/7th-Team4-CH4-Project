// Fill out your copyright notice in the Description page of Project Settings.


#include "SMCustomizeWidget.h"

#include "Character/SMPlayerCharacter.h"
#include "Character/SMPlayerController.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Core/SMPlayerState.h"

void USMCustomizeWidget::CustomizeSetup()
{
	// PC/Character 한 번씩만 가져와서 재사용
	ASMPlayerController* PC = GetSMPlayerController();
	if (IsValid(PC) == false) return;

	ASMPlayerState* PS = PC->GetPlayerState<ASMPlayerState>();
	if (IsValid(PS) == false) return;

	PreviousWeaponIdx = PS->GetSelectedWeaponIndex();
	PreviousMatIdx = PS->GetSelectedMaterialIndex();
	CurrentWeaponIdx = PreviousWeaponIdx;
	CurrentMatIdx = PreviousMatIdx;
	
	if (IsValid(CurrentMaterialNumber) == true)
	{
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx+1)));
	}
	
	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;
	
	Character->SetCustomizeMode(true);	
}

bool USMCustomizeWidget::Initialize()
{
	if (Super::Initialize() == false) return false;

	if (IsValid(WeaponButton0) == true)
	{
		WeaponButton0->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnWeaponButton0Clicked);
	}
	if (IsValid(WeaponButton1) == true)
	{
		WeaponButton1->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnWeaponButton1Clicked);
	}
	if (IsValid(WeaponButton2) == true)
	{
		WeaponButton2->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnWeaponButton2Clicked);
	}

	if (IsValid(SkillButton0) == true)
	{
		SkillButton0->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnSkillButton0Clicked);
	}
	if (IsValid(SkillButton1) == true)
	{
		SkillButton1->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnSkillButton1Clicked);
	}
	if (IsValid(SkillButton2) == true)
	{
		SkillButton2->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnSkillButton2Clicked);
	}
	if (IsValid(SkillButton3) == true)
	{
		SkillButton3->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnSkillButton3Clicked);
	}

	if (IsValid(MaterialPrevButton) == true)
	{
		MaterialPrevButton->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnMaterialPrevClicked);
	}
	if (IsValid(MaterialNextButton) == true)
	{
		MaterialNextButton->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnMaterialNextClicked);
	}

	if (IsValid(ApplyButton) == true)
	{
		ApplyButton->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnApplyClicked);
	}
	if (IsValid(CancelButton) == true)
	{
		CancelButton->OnClicked.AddDynamic(this, &USMCustomizeWidget::OnCancelClicked);
	}

	return true;
}

void USMCustomizeWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

//================================
// 무기 선택
//================================

void USMCustomizeWidget::OnWeaponButton0Clicked() { SelectWeapon(0); }

void USMCustomizeWidget::OnWeaponButton1Clicked() { SelectWeapon(1); }

void USMCustomizeWidget::OnWeaponButton2Clicked() { SelectWeapon(2); }

void USMCustomizeWidget::SelectWeapon(int32 WeaponIndex)
{
	CurrentWeaponIdx = WeaponIndex;

	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;

	Character->ApplyCustomizationLocal(CurrentWeaponIdx, CurrentMatIdx);
}

//================================
// 스킬 선택
//================================

void USMCustomizeWidget::OnSkillButton0Clicked()
{
}

void USMCustomizeWidget::OnSkillButton1Clicked()
{
}

void USMCustomizeWidget::OnSkillButton2Clicked()
{
}

void USMCustomizeWidget::OnSkillButton3Clicked()
{
}

//================================
// 머티리얼 선택
//================================

void USMCustomizeWidget::OnMaterialPrevClicked()
{
	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;

	int32 Num = Character->GetMaterialOptionNum();
	if (Num == 0) return;

	CurrentMatIdx = (CurrentMatIdx - 1 + Num) % Num;
	Character->ApplyCustomizationLocal(CurrentWeaponIdx, CurrentMatIdx);
	
	if (IsValid(CurrentMaterialNumber) == true)
	{
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx+1)));
	}
}

void USMCustomizeWidget::OnMaterialNextClicked()
{
	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;

	int32 Num = Character->GetMaterialOptionNum();
	if (Num == 0) return;

	CurrentMatIdx = (CurrentMatIdx + 1) % Num;
	Character->ApplyCustomizationLocal(CurrentWeaponIdx, CurrentMatIdx);
	
	if (IsValid(CurrentMaterialNumber) == true)
	{
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx+1)));
	}
}

//================================
// 확정 / 취소
//================================

void USMCustomizeWidget::OnApplyClicked()
{
	ASMPlayerController* PC = GetSMPlayerController();
	if (IsValid(PC) == false) return;

	//이 시점에만 ServerRPC -> OnRep_으로 다른 플레이어에게도 복제
	PC->ServerRPCSetCustomization(CurrentWeaponIdx, CurrentMatIdx);

	CloseWidget();
}

void USMCustomizeWidget::OnCancelClicked()
{
	// 이전 값으로 로컬 복원 (RPC 없음)
	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == true)
	{
		Character->ApplyCustomizationLocal(PreviousWeaponIdx, PreviousMatIdx);
	}

	CloseWidget();
}

void USMCustomizeWidget::CloseWidget()
{
	// 카메라/이동 복원
	ASMPlayerCharacter* Char = GetSMPlayerCharacter();
	if (IsValid(Char))
	{
		Char->SetCustomizeMode(false);
	}

	// PC 경유로 위젯 닫기 + LobbyWidget 복원 + InputMode 복구
	ASMPlayerController* PC = GetSMPlayerController();
	if (IsValid(PC) == false) return;

	PC->CloseCustomizeWidget();
}

ASMPlayerController* USMCustomizeWidget::GetSMPlayerController() const
{
	return Cast<ASMPlayerController>(GetOwningPlayer());
}

ASMPlayerCharacter* USMCustomizeWidget::GetSMPlayerCharacter() const
{
	ASMPlayerController* PC = GetSMPlayerController();
	if (IsValid(PC) == false) return nullptr;

	return Cast<ASMPlayerCharacter>(PC->GetPawn());
}
