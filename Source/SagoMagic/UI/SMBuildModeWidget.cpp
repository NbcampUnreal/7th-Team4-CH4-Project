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
		const int32 ChildCount = SlotContainer->GetChildrenCount();
		for (int32 i = 0; i < ChildCount; ++i)
		{
			UWidget* Child = SlotContainer->GetChildAt(i);
			UBorder* FoundBorder = nullptr;
			
			if (USizeBox* SizeBox = Cast<USizeBox>(Child))
			{
				if (SizeBox->GetChildrenCount() > 0)
				{
					FoundBorder = Cast<UBorder>(SizeBox->GetChildAt(0));
				}
			}
			else if (UBorder* DirectBorder = Cast<UBorder>(Child))
			{
				FoundBorder = DirectBorder;
			}
			
			if (FoundBorder)
			{
				SlotBorders.Add(FoundBorder);
				
				FoundBorder->SetBrushColor(DefaultColor); 
			}
		}
	}
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
	
	UpdateSlotHighlight(Message.CurrentSlotIndex);
}

void USMBuildModeWidget::UpdateSlotHighlight(int32 SelectedIndex)
{
	if (SlotBorders.IsEmpty())
	{
		return;
	}

	for (int32 i = 0; i < SlotBorders.Num(); ++i)
	{
		if (UBorder* Border = SlotBorders[i])
		{
			if (i == SelectedIndex)
			{
				Border->SetBrushColor(SelectedColor);
			}
			else
			{
				Border->SetBrushColor(DefaultColor);
			}
		}
	}
}