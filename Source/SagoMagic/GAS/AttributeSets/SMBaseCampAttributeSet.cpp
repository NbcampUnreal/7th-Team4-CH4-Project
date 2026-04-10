#include "SMBaseCampAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Core/SMGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Core/SMGameMode.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/SMGameplayMessages.h"
#include "GameplayTags/UI/SMUITag.h"

USMBaseCampAttributeSet::USMBaseCampAttributeSet()
{
	InitHealth(100.f);
	InitMaxHealth(100.f);
}

void USMBaseCampAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMBaseCampAttributeSet, Health, OldHealth);
	//TODO 현 : HP가 변경될 때마다 UI에 쏴주는 부분
	// 클라이언트 UI 업데이트를 위해 방송
	if (UWorld* World = GetWorld())
	{
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(World);
		
		FBaseCampMsg Msg;
		Msg.EventType = (GetHealth() <= 0.0f) ? EBaseCampEvent::Destroyed : EBaseCampEvent::Attacked;
		Msg.BaseCampHP = GetHealth() / GetMaxHealth();
		
		Msg.CurrentHP = GetHealth();
		Msg.MaxHP = GetMaxHealth();
		
		MessageSystem.BroadcastMessage(SMUITag::Event_BaseCamp, Msg);
	}
	
}

void USMBaseCampAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(USMBaseCampAttributeSet, MaxHealth, OldMaxHealth);
}

void USMBaseCampAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(USMBaseCampAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(USMBaseCampAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
}

void USMBaseCampAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		const float NewHealth = FMath::Clamp(GetHealth(),0,GetMaxHealth());
		SetHealth(NewHealth);
		if (NewHealth <=0)
		{
			UE_LOG(LogTemp, Log, TEXT("[BaseCampAttributeSet] HP 0 - 패배 처리"));
			if (AActor* OwnerActor = Cast<AActor>(GetOwningActor()))
			{
				if (UWorld* World = OwnerActor->GetWorld())
				{
					if (ASMGameMode* GM = World->GetAuthGameMode<ASMGameMode>())
					{
						GM->OnBaseCampDestroyed();
					}
				}
			}
		}
	}
}

void USMBaseCampAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
}
