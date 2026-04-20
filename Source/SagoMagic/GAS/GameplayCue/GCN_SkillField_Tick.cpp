#include "GAS/GameplayCue/GCN_SkillField_Tick.h"
#include "Core/DataManager/SMSoundManager.h"

bool UGCN_SkillField_Tick::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	Super::OnExecute_Implementation(MyTarget, Parameters);

	// IsValid=false 여도 포인터가 살아있으면 위치 접근 가능
	const FVector Location = !Parameters.Location.IsNearlyZero()
		? Parameters.Location
		: (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector);

	if (Location.IsNearlyZero()) return false;

	UWorld* World = MyTarget ? MyTarget->GetWorld() : nullptr;
	if (!World) return false;

	USMSoundManager* SM = USMSoundManager::Get(this);
	if (IsValid(SM) == true)
	{
		SM->PlaySoundAtLocation(TEXT("FieldTick"), Location);
	}
	return true;
}
