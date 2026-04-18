#include "UI/SMBuildingHPBarWidget.h"
#include "Components/ProgressBar.h"

void USMBuildingHPBarWidget::SetMaxHP(float InMaxHP)
{
	CachedMaxHP = FMath::Max(1.f, InMaxHP);
}

void USMBuildingHPBarWidget::UpdateHPBar(float CurrentHP, float MaxHP)
{
	CachedMaxHP   = FMath::Max(1.f, MaxHP);
	TargetPercent = FMath::Clamp(CurrentHP / CachedMaxHP, 0.f, 1.f);
}

void USMBuildingHPBarWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);

	if (!FMath::IsNearlyEqual(CurrentPercent, TargetPercent, 0.001f))
	{
		CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, DeltaTime, InterpSpeed);
	}
	else
	{
		CurrentPercent = TargetPercent;
	}

	if (ProgressBar_BuildingHP)
	{
		ProgressBar_BuildingHP->SetPercent(CurrentPercent);
	}
}