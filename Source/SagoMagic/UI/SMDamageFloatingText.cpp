#include "SMDamageFloatingText.h"
#include "Components/WidgetComponent.h"
#include "SMDamageTextWidget.h"

ASMDamageFloatingText::ASMDamageFloatingText()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false; // 서버에서 복제되지 X
	
	DamageWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageWidget"));
	RootComponent = DamageWidgetComp;

	// 위젯이 월드 공간이 아닌 Screen 공간에 그려지도록 설정
	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ASMDamageFloatingText::BeginPlay()
{
	Super::BeginPlay();

	if (DamageWidgetComp && PendingDamage > 0.f)
	{
		USMDamageTextWidget* DamageWidget = Cast<USMDamageTextWidget>(
			DamageWidgetComp->GetUserWidgetObject());
		if (DamageWidget)
		{
			DamageWidget->SetDamageText(PendingDamage);
		}
	}
	
	// 겹침 방지
	float RandomX = FMath::RandRange(-50.0f, 50.0f);
	float RandomY = FMath::RandRange(-50.0f, 50.0f);
	float RandomZ = FMath::RandRange(-20.0f, 20.0f);
	FVector NewLocation = GetActorLocation() + FVector(RandomX, RandomY, RandomZ);
	SetActorLocation(NewLocation);
	// 지정된 시간이 지나면 자동으로 월드에서 제거되도록 설정
	SetLifeSpan(LifeSpan);
}

void ASMDamageFloatingText::SetDamageValue(float DamageAmount)
{
	PendingDamage = DamageAmount;
	
	if (DamageWidgetComp)
	{
		USMDamageTextWidget* DamageWidget = Cast<USMDamageTextWidget>(DamageWidgetComp->GetUserWidgetObject());
        
		if (DamageWidget)
		{
			DamageWidget->SetDamageText(DamageAmount);
		}
	}
}
