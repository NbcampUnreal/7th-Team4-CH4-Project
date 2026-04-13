#include "SMFenceBuilding.h"
#include "Net/UnrealNetwork.h"
ASMFenceBuilding::ASMFenceBuilding()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASMFenceBuilding::ConvertToCorner(float Yaw)
{
	if (!HasAuthority()) return;
	if (bIsCorner) return;
	if (!CornerMesh) return;
	
	MeshComponent->SetStaticMesh(CornerMesh);
	SetActorRotation(FRotator(0.f, Yaw, 0.f));
	
	bIsCorner = true;
	CornerYaw = Yaw;
}

void ASMFenceBuilding::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ASMFenceBuilding, bIsCorner);
	DOREPLIFETIME(ASMFenceBuilding, CornerYaw);
}

void ASMFenceBuilding::OnRep_bIsCorner()
{
	if (bIsCorner && CornerMesh)
	{
		MeshComponent->SetStaticMesh(CornerMesh);
		SetActorRotation(FRotator(0.f, CornerYaw, 0.f));
	}
}

