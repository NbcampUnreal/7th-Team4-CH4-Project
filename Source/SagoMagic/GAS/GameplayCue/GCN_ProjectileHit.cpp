#include "GAS/GameplayCue/GCN_ProjectileHit.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

bool UGCN_ProjectileHit::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	Super::OnExecute_Implementation(MyTarget, Parameters);

	if (!HitNiagaraSystem) return false;

	// FGameplayCueParameters에서 충돌 위치/방향 추출
	const FVector Location = Parameters.Location.IsNearlyZero()
		? (MyTarget ? MyTarget->GetActorLocation() : FVector::ZeroVector)
		: Parameters.Location;

	const FRotator Rotation = Parameters.Normal.IsNearlyZero()
		? FRotator::ZeroRotator
		: Parameters.Normal.Rotation();

	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		MyTarget, HitNiagaraSystem, Location, Rotation);
	//Niagara User Parameter 이름과 일치해야 함
	if (NiagaraComp)
	{
		NiagaraComp->SetVariableLinearColor(FName("Color_REF"),     Color_REF);
		NiagaraComp->SetVariableLinearColor(FName("Color_Smoke"),   Color_Smoke);
		NiagaraComp->SetVariableLinearColor(FName("Color_Sparks1"), Color_Sparks1);
		NiagaraComp->SetVariableLinearColor(FName("Color_Spiral1"), Color_Spiral1);
		NiagaraComp->SetVariableLinearColor(FName("Color_Trail"),   Color_Trail);
		NiagaraComp->SetVariableFloat(FName("Scale_All"),           Scale_All);
	}

	return true;
}
