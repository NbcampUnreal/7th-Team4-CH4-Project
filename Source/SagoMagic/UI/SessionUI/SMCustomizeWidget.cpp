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

	const TSoftObjectPtr<USMItemDefinition>& PrevSkill = PS->GetSelectedLobbySkillDef();
	CurrentSkillIndex = 0;
	for (int32 i = 0; i < ChekSkillDefinitions.Num(); i++)
	{
		if (ChekSkillDefinitions[i] == PrevSkill)
		{
			CurrentSkillIndex = i;
			break;
		}
	}
	
	PreviousWeaponIdx = PS->GetSelectedWeaponIndex();
	PreviousMatIdx = PS->GetSelectedMaterialIndex();
	CurrentWeaponIdx = PreviousWeaponIdx;
	CurrentMatIdx = PreviousMatIdx;

	if (IsValid(CurrentMaterialNumber) == true)
	{
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx + 1)));
	}

	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;

	Character->SetCustomizeMode(true);
	
	// 위젯이 열릴 때 기존에 선택되어 있던 인덱스를 기반으로 초기 하이라이트 적용
	UpdateWeaponButtonHighlight();
	UpdateSkillButtonHighlight();
}

bool USMCustomizeWidget::Initialize()
{
	if (Super::Initialize() == false) return false;
	
	WeaponButtons = { WeaponButton0, WeaponButton1, WeaponButton2 };
	SkillButtons = { SkillButton0, SkillButton1, SkillButton2, SkillButton3 };
	
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
// 시각적 피드백
//================================

void USMCustomizeWidget::UpdateWeaponButtonHighlight()
{
	for (int32 i = 0; i < WeaponButtons.Num(); ++i)
	{
		if (IsValid(WeaponButtons[i]))
		{
			WeaponButtons[i]->SetBackgroundColor(i == CurrentWeaponIdx ? SelectedButtonColor : DefaultButtonColor);
		}
	}
}

void USMCustomizeWidget::UpdateSkillButtonHighlight()
{
	for (int32 i = 0; i < SkillButtons.Num(); ++i)
	{
		if (IsValid(SkillButtons[i]))
		{
			SkillButtons[i]->SetBackgroundColor(i == CurrentSkillIndex ? SelectedButtonColor : DefaultButtonColor);
		}
	}
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

	// 무기가 선택될 때마다 UI 하이라이트 상태 갱신
	UpdateWeaponButtonHighlight();
	
	ASMPlayerCharacter* Character = GetSMPlayerCharacter();
	if (IsValid(Character) == false) return;

	Character->ApplyCustomizationLocal(CurrentWeaponIdx, CurrentMatIdx);
}

//================================
// 스킬 선택
//================================

void USMCustomizeWidget::OnSkillButton0Clicked() { SelectSkill(0); }

void USMCustomizeWidget::OnSkillButton1Clicked() { SelectSkill(1); }

void USMCustomizeWidget::OnSkillButton2Clicked() { SelectSkill(2); }

void USMCustomizeWidget::OnSkillButton3Clicked() { SelectSkill(3); }

void USMCustomizeWidget::SelectSkill(int32 Index)
{
	if (Index < 0 || Index > 3) return;
	CurrentSkillIndex = Index;
	
	UpdateSkillButtonHighlight();
	
	UE_LOG(LogTemp,Warning,TEXT("Selected Skill Index: %d"), CurrentSkillIndex);
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
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx + 1)));
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
		CurrentMaterialNumber->SetText(FText::FromString(FString::Printf(TEXT("%d"), CurrentMatIdx + 1)));
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
	
	//선택된 스킬 인덱스만 전송 (서버가 GM의 DA 조회 후 인벤토리에 스킬 전송)
	PC->ServerRPCSelectLobbySkill(CurrentSkillIndex);
	UE_LOG(LogTemp,Warning,TEXT("Selected Skill Index: %d Send to Server"), CurrentSkillIndex);

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
