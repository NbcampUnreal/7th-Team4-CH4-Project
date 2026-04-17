#include "GAS/GameplayCue/GCN_SkillField_Tick.h"
#include "Kismet/GameplayStatics.h"

bool UGCN_SkillField_Tick::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	Super::OnExecute_Implementation(MyTarget, Parameters);

	if (!TickSound) return false;

	const FVector Location = Parameters.Location.IsNearlyZero()
		? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector)
		: Parameters.Location;

	UGameplayStatics::PlaySoundAtLocation(MyTarget, TickSound, Location);
	return true;
}
