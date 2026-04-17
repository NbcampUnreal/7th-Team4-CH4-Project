#include "UI/SMBuildModeWidget.h"
#include "GameplayTags/UI/SMUITag.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"

void USMBuildModeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MsgSys = UGameplayMessageSubsystem::Get(this);
	BuildModeListenerHandle = MsgSys.RegisterListener<FBuildModeMsg>(
		SMUITag::Event_BuildMode, this, &ThisClass::OnBuildModeMessage);

	SlotBorders.Empty();
	if (SlotContainer)
	{
		for (int32 i = 0; i < SlotContainer->GetChildrenCount(); ++i)
		{
			UWidget* Child = SlotContainer->GetChildAt(i);
			UE_LOG(LogTemp, Warning, TEXT("[BuildModeUI] %d번 자식 클래스: %s"), i, *Child->GetClass()->GetName());

			// SizeBox인 경우
			if (USizeBox* SizeBox = Cast<USizeBox>(Child))
			{
				if (SizeBox->GetChildrenCount() > 0)
				{
					UWidget* GrandChild = SizeBox->GetChildAt(0);
					if (UBorder* Border = Cast<UBorder>(GrandChild))
					{
						SlotBorders.Add(Border);
					}
				}
			}
			// 바로 Border가 들어간 경우
			else if (UBorder* DirectBorder = Cast<UBorder>(Child))
			{
				SlotBorders.Add(DirectBorder);
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[BuildModeUI] 최종 발견한 슬롯(Border) 개수 : %d"), SlotBorders.Num());
	SetVisibility(ESlateVisibility::Collapsed);
}

void USMBuildModeWidget::NativeDestruct()
{
	if (BuildModeListenerHandle.IsValid())
	{
		BuildModeListenerHandle.Unregister();
	}
	Super::NativeDestruct();
}

void USMBuildModeWidget::OnBuildModeMessage(FGameplayTag Channel, const FBuildModeMsg& Message)
{
	if (!Message.bIsActive)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	
	// Q, E로 슬롯 인덱스가 변경될 때마다 하이라이트 함수 호출
	UpdateSlotHighlight(Message.CurrentSlotIndex);
}

void USMBuildModeWidget::UpdateSlotHighlight(int32 SelectedIndex)
{
	UE_LOG(LogTemp, Error, TEXT("[BuildModeUI] 색상 변경 요청! 들어온 인덱스 : %d"), SelectedIndex);
	
	for (int32 i = 0; i < SlotBorders.Num(); ++i)
	{
		if (SlotBorders[i])
		{
			if (i == SelectedIndex)
			{
				SlotBorders[i]->SetBrushColor(SelectedColor);
			}
			else
			{
				SlotBorders[i]->SetBrushColor(DefaultColor);
			}
		}
	}
}