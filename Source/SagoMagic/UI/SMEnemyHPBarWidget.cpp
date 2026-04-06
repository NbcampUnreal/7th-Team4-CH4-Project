#include "SMEnemyHPBarWidget.h"

void USMEnemyHPBarWidget::UpdateHPBar(float Percent)
{
    TargetPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
}

void USMEnemyHPBarWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);

    if (!FMath::IsNearlyEqual(CurrentPercent, TargetPercent, 0.001f))
    {
        CurrentPercent = FMath::FInterpTo(CurrentPercent, TargetPercent, DeltaTime, InterpSpeed);

        if (EnemyHPBar)
        {
            EnemyHPBar->SetPercent(CurrentPercent);
        }
        
        else
        {
            // 거의 같아지면 완전히 일치시켜서 불필요한 계산 종료
            CurrentPercent = TargetPercent;
            if (EnemyHPBar)
            {
                EnemyHPBar->SetPercent(CurrentPercent);
            }
        }
    }
}
