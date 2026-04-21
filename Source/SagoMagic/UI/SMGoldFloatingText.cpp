#include "SMGoldFloatingText.h"

#include "SMGoldTextWidget.h"
#include "Components/WidgetComponent.h"

ASMGoldFloatingText::ASMGoldFloatingText()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	
	GoldWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("GoldWidget"));
	RootComponent = GoldWidgetComp;
	GoldWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ASMGoldFloatingText::BeginPlay()
{
	Super::BeginPlay();
	
	if (GoldWidgetComp)
	{
		USMGoldTextWidget* GoldWidget = Cast<USMGoldTextWidget>(
			GoldWidgetComp->GetUserWidgetObject());
		if (GoldWidget)
		{
			GoldWidget->SetGoldText(PendingGold);
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

void ASMGoldFloatingText::SetGoldValue(float GoldDelta)
{
	PendingGold = GoldDelta;
	if (GoldWidgetComp)
	{
		USMGoldTextWidget* GoldWidget = Cast<USMGoldTextWidget>(
			GoldWidgetComp->GetUserWidgetObject());
		if (GoldWidget)
		{
			GoldWidget->SetGoldText(GoldDelta);
		}
	}
}
