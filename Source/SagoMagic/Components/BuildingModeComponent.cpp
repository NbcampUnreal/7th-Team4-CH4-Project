#include "BuildingModeComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SagoMagic.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Building/SMBaseBuilding.h"
#include "Building/SMBuildPlaceTargetData.h"
#include "Building/SMFenceBuilding.h"
#include "Core/SMPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/Character/SMCharacterTag.h"
#include "Kismet/GameplayStatics.h"

USMBuildingModeComponent::USMBuildingModeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	BuildPlaceEventTag = SMCharacterTag::Ability_Build_Place;
	SetIsReplicatedByDefault(true);
}

void USMBuildingModeComponent::EnableBuildMode()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;
	
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!Subsystem || !BuildIMC) return;
	
	Subsystem->AddMappingContext(BuildIMC, 1);
	bIsBuildMode = true;
	SetComponentTickEnabled(true);
	
	FBuildModeMsg Msg;
	Msg.bIsActive = true;
	Msg.CurrentSlotIndex = CurrentSlotIndex;
	Msg.TotalSlotCount = CachedBuildingData.Num();
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(SMUITag::Event_BuildMode, Msg);
}

void USMBuildingModeComponent::DisableBuildMode()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem && BuildIMC)
	{
		Subsystem->RemoveMappingContext(BuildIMC);
	}
	
	bIsBuildMode = false;
	bIsWaitingForEndPoint = false;
	SetComponentTickEnabled(false);
	ClearGhostActors();
	
	FBuildModeMsg Msg;
	Msg.bIsActive = false;
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(SMUITag::Event_BuildMode, Msg);
}

void USMBuildingModeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!GridManager)
	{
		GridManager = Cast<ASMGridManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ASMGridManager::StaticClass())
		);
	}
}

void USMBuildingModeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsBuildMode)
	{
		UpdateGhostTransform();
	}
}

void USMBuildingModeComponent::SetupInputBindings()
{
	if (bInputBound) return;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->InputComponent) return;
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent);
	if (!Input) return;

	LoadBuildingDataTable();
	
	if (PlaceAction)
	{
		Input->BindAction(PlaceAction, ETriggerEvent::Started, this,
			&USMBuildingModeComponent::OnPlaceBuilding);
	}
	if (RotateAction)
	{
		Input->BindAction(RotateAction, ETriggerEvent::Triggered, this,
			&USMBuildingModeComponent::OnRotateBuilding);
	}
	if (CycleAction)
	{
		Input->BindAction(CycleAction, ETriggerEvent::Started, this,
			&USMBuildingModeComponent::OnCycleBuilding);
	}
	bInputBound = true;
}

void USMBuildingModeComponent::OnPlaceBuilding(const FInputActionValue& Value)
{
	if (!bIsBuildMode || !GridManager) return;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;
	
	FHitResult Hit;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, true, Hit)) return;
	if (Hit.ImpactNormal.Z <= 0.9f) return;
	
	FIntPoint ClickedGrid = GridManager->WorldToGrid(Hit.Location);
	
	//첫번째 클릭
	if (!bIsWaitingForEndPoint)
	{
		SM_LOG(this, LogSM, Warning, TEXT("1번째 클릭"));
		float DistXY = FVector::Dist2D(OwnerPawn->GetActorLocation(), Hit.Location);
		if (DistXY > MaxBuildDistance) return;
		
		FenceStartGrid = ClickedGrid;
		bIsWaitingForEndPoint = true;
		return;
	}
	
	const FSMBuildingData* Data = GetCurrentBuildingData();
	if (!Data)
	{
		SM_LOG(this, LogSM, Warning, TEXT("2번째 클릭 - Data 없음"));
		return;
	}
	
	if (!CheckPlacementValidity(Hit.Location,*Data))
	{
		SM_LOG(this, LogSM, Warning, TEXT("2번째 클릭 - 유효성 실패"));
		return;
	}
	
	TArray<FIntPoint> Path = GridManager->FindPath(FenceStartGrid, ClickedGrid);
	if (Path.IsEmpty())
	{
		SM_LOG(this, LogSM, Warning, TEXT("2번째 클릭 - 경로 없음 Start(%d,%d) End(%d,%d)"),
		FenceStartGrid.X, FenceStartGrid.Y, ClickedGrid.X, ClickedGrid.Y);
		return;
	}
	TArray<FSMCellPlaceInfo> CellInfos;
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FSMCellPlaceInfo Cellinfo;
		Cellinfo.Grid = Path[i];
		
		FSMCornerInfo CornerInfo = GetEffectiveCornerInfo(Path, i);
		Cellinfo.bIsCorner = CornerInfo.bIsCorner;

		if (CornerInfo.bIsCorner)
		{
			Cellinfo.Yaw = CornerInfo.Yaw;
		}
		else
		{
			FIntPoint Dir;
			if (i >0)
				Dir = Path[i] - Path[i-1];
			else if (Path.Num() > 1)
				Dir = Path[1] - Path[0];
			else
				Dir = FIntPoint(1, 0);
			Cellinfo.Yaw = (Dir.Y != 0) ? 90.f : 0.f;
		}
		CellInfos.Add(Cellinfo);
	}
	SM_LOG(this, LogSM, Warning, TEXT("2번째 클릭 - ServerRPC 호출 Path %d개"), Path.Num());
	ServerRPC_RequestPlaceBuilding(CellInfos, Data->BuildingType);
	
	bIsWaitingForEndPoint = false;
	LastHoverGrid = FIntPoint(-1, -1);
	ClearGhostActors();
	
}

void USMBuildingModeComponent::OnRotateBuilding(const FInputActionValue& Value)
{
	if (!bIsBuildMode) return;
	float Input = Value.Get<float>();
	if (Input > 0)
	{
		CurrentRotationIndex = (CurrentRotationIndex + 1) % 4;
	}
	else
	{
		CurrentRotationIndex = (CurrentRotationIndex -1 + 4) % 4;
	}
	LastHoverGrid = FIntPoint(-1, -1);
}

void USMBuildingModeComponent::OnCycleBuilding(const FInputActionValue& Value)
{
	if (!bIsBuildMode) return;
	if (CachedBuildingData.IsEmpty()) return;
	
	float Input = Value.Get<float>();
	if (Input < 0)
	{
		CurrentSlotIndex = (CurrentSlotIndex + 1) % CachedBuildingData.Num();
	}
	else
	{
		CurrentSlotIndex = (CurrentSlotIndex -1 + CachedBuildingData.Num()) % CachedBuildingData.Num();
	}
	
	CurrentRotationIndex = 0;
	LastHoverGrid = FIntPoint(-1, -1);
	ClearGhostActors();
	
	FBuildModeMsg Msg;
	Msg.bIsActive = true;
	Msg.CurrentSlotIndex = CurrentSlotIndex;
	Msg.TotalSlotCount = CachedBuildingData.Num();
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(SMUITag::Event_BuildMode, Msg);
}

void USMBuildingModeComponent::UpdateGhostTransform()
{
	if (!GetWorld()) return;
	if (!GridManager)
	{
		SM_LOG(this, LogSM, Warning, TEXT("[Ghost] GridManager 없음"));
		return;
	}
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		SM_LOG(this, LogSM, Warning, TEXT("[Ghost] OwnerPawn 없음"));
		return;
	}
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC)
	{
		SM_LOG(this, LogSM, Warning, TEXT("[Ghost] PC 없음"));
		return;
	}
	
	FHitResult Hit;
	bool bHit = PC->GetHitResultUnderCursor(ECC_WorldStatic, false, Hit);
	
	if (!bHit || Hit.ImpactNormal.Z <= 0.9f)
	{
		ClearGhostActors();
		LastHoverGrid = FIntPoint(-1, -1);
		bIsPlacementValid = false;
		return;
	}
	
	FIntPoint HoverGrid = GridManager->WorldToGrid(Hit.Location);
	
	if (HoverGrid == LastHoverGrid) return;
	LastHoverGrid = HoverGrid;
	
	const FSMBuildingData* Data = GetCurrentBuildingData();
	if (!Data || !Data->BuildingClass)
	{
		ClearGhostActors();
		return;
	}
	
	if (!Data->BuildingClass->IsChildOf(ASMBaseBuilding::StaticClass()))
	{
		SM_LOG(this, LogSM, Warning, TEXT("[Ghost] 잘못된 BuildingClass: %s"),
			*Data->BuildingClass->GetName());
		ClearGhostActors();
		return;
	}
	
	//경로 계산
	//첫 클릭 전 : 커서 위치 단일 셀 1개
	//첫 클릭 후 : 시작점 ->커서까지 직선 경로
	TArray<FIntPoint> Path;
	if (!bIsWaitingForEndPoint)
	{
		Path.Add(HoverGrid);
	}
	else
	{
		Path = GridManager->FindPath(FenceStartGrid, HoverGrid);
	}
	
	//코너 정보 한 번에 계산
	TArray<FSMCornerInfo> CornerInfos;
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		CornerInfos.Add(bIsWaitingForEndPoint
			? GetEffectiveCornerInfo(Path, i)
			: FSMCornerInfo{false, 0.f});
	}
	
	//고스트 배열 재구성
	ClearGhostActors();
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
			Data->BuildingClass, FTransform::Identity);
		ASMBaseBuilding* Ghost = Cast<ASMBaseBuilding>(SpawnedActor);
		if (!Ghost)
		{
			if (SpawnedActor) SpawnedActor->Destroy();
			continue;
		}
		
		Ghost->SetReplicates(false);
		Ghost->SetActorEnableCollision(false);
		
		if (CornerInfos[i].bIsCorner)
			ConvertGhostToCorner(Ghost, CornerInfos[i].Yaw);
		
		GhostActors.Add(Ghost);
		
	}

	for (int32 i = 0; i < Path.Num() && i < GhostActors.Num(); ++i)
	{
		ASMBaseBuilding* Ghost = GhostActors[i];
		if (!Ghost) continue;

		if (!GridManager->IsCellEmpty(Path[i].X, Path[i].Y))
		{
			Ghost->SetActorHiddenInGame(true);
			continue;
		}
		
		float Yaw;
		if (!bIsWaitingForEndPoint)
		{
			//첫 클릭 전 휠 회전 적용
			Yaw = CurrentRotationIndex * 90.f;
		}
		else if (CornerInfos[i].bIsCorner)
		{
			Yaw = CornerInfos[i].Yaw;
		}
		else
		{
			FIntPoint Dir;
			if (i == 0 && Path.Num() >1)
				Dir = Path[1] -Path[0];
			else if (i > 0)
				Dir = Path[i] - Path[i-1];
			else
				Dir = FIntPoint(1,0);
			Yaw = (Dir.Y != 0) ? 90.f : 0.f;
		}
		
		Ghost->SetActorRotation(FRotator(0.f, Yaw, 0.f));
		
		FVector TargetPos = GridManager->GridToWorld(Path[i].X, Path[i].Y);
		TargetPos.Z = Hit.ImpactPoint.Z;
		Ghost->SetActorLocation(TargetPos);
		
		Ghost->SetActorHiddenInGame(false);
	}
	bIsPlacementValid = CheckPlacementValidity(Hit.Location, *Data);
	UpdateGhostMaterials();
}

void USMBuildingModeComponent::UpdateGhostMaterials()
{
	UMaterialInterface* Mat = bIsPlacementValid ? ValidMaterial : InValidMaterial;
	if (!Mat) return;

	for (AActor* Ghost : GhostActors)
	{
		if (!Ghost) continue;
		TArray<UMeshComponent*> Meshes;
		Ghost->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
			{
				Mesh->SetMaterial(i, Mat);
			}
		}
	}
}

void USMBuildingModeComponent::ClearGhostActors()
{
	for (AActor* Ghost : GhostActors)
	{
		if (Ghost) Ghost->Destroy();
	}
	GhostActors.Empty();
}

bool USMBuildingModeComponent::CheckPlacementValidity(const FVector& Location, const FSMBuildingData& Data)
{
	if (!GridManager) return false;
	
	AActor* Owner = GetOwner();
	if (Owner)
	{
		float DistXY = FVector::Dist2D(Owner->GetActorLocation(),Location);
		if (DistXY > MaxBuildDistance) return false;
	}
	
	// 1) 그리드 데이터 검사: 범위 + 점유 여부
	FIntPoint ActualSize = Data.GridSize;
	if (CurrentRotationIndex % 2 != 0)
	{
		Swap(ActualSize.X, ActualSize.Y);
	}
	
	// Location이 건물 중심이므로 float 기반으로 BaseX/BaseY 직접 계산 (FloorToInt 오차 방지)
	float RelX = Location.X - GridManager->GridOrigin.X;
	float RelY = Location.Y - GridManager->GridOrigin.Y;
	int32 BaseX = FMath::FloorToInt(RelX / GridManager->CellSize - ActualSize.X * 0.5f);
	int32 BaseY = FMath::FloorToInt(RelY / GridManager->CellSize - ActualSize.Y * 0.5f);

	if (!GridManager->CanPlaceBuilding(BaseX, BaseY, Data.GridSize, CurrentRotationIndex))
	{
		return false;
	}

	// 2) 물리 오버랩 검사: 다른 Pawn/Dynamic 액터와 겹치는지
	FVector BoxExtent;
	BoxExtent.X = (ActualSize.X * GridManager->CellSize) * 0.5f - 2.0f;
	BoxExtent.Y = (ActualSize.Y * GridManager->CellSize) * 0.5f - 2.0f;
	BoxExtent.Z = 80.0f;

	FCollisionQueryParams QParams;
	for (AActor* Ghost : GhostActors)
	{
		QParams.AddIgnoredActor(Ghost);
	}
	QParams.AddIgnoredActor(GetOwner());

	FCollisionObjectQueryParams ObjParams;
	ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjParams.AddObjectTypesToQuery(ECC_Pawn);

	FVector Center = Location + FVector(0, 0, BoxExtent.Z + 10.0f);
	bool bOverlap = GetWorld()->OverlapAnyTestByObjectType(
		Center, FQuat::Identity, ObjParams,
		FCollisionShape::MakeBox(BoxExtent), QParams);

	return !bOverlap;
	
}

void USMBuildingModeComponent::ConvertGhostToCorner(ASMBaseBuilding* Ghost, float Yaw)
{
	if (!Ghost) return;
	if (ASMFenceBuilding* FenceGhost = Cast<ASMFenceBuilding>(Ghost))
	{
		FenceGhost->ConvertToCornerPreview(Yaw);
	}
}

FSMCornerInfo USMBuildingModeComponent::GetCornerInfo(const TArray<FIntPoint>& Path, int32 i) const
{
	if (i == 0 || i >= Path.Num() -1) return {false, 0.f};
	
	FIntPoint InDir = Path[i] - Path[i -1];
	FIntPoint OutDir = Path[i+1] - Path[i];
	
	if (InDir == OutDir) return {false, 0.f};
	
	float Yaw = GetCornerYawFromConnections(InDir, OutDir);
	return {true, Yaw};
}

void USMBuildingModeComponent::SendPlaceEvent(const TArray<FIntPoint>& Path, FIntPoint EndGrid)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	const FSMBuildingData* Data = GetCurrentBuildingData();
	if (!Data) return;
	
	FSMBuildPlaceTargetData* TargetData = new FSMBuildPlaceTargetData();
	TargetData->BuildingType = Data->BuildingType;
	
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FSMCellPlaceInfo Cellinfo;
		Cellinfo.Grid = Path[i];
		
		FSMCornerInfo CornerInfo = GetEffectiveCornerInfo(Path, i);
		Cellinfo.bIsCorner = CornerInfo.bIsCorner;

		if (CornerInfo.bIsCorner)
		{
			Cellinfo.Yaw = CornerInfo.Yaw;
		}
		else
		{
			FIntPoint Dir;
			if (i >0)
				Dir = Path[i] - Path[i-1];
			else if (Path.Num() > 1)
				Dir = Path[1] - Path[0];
			else
				Dir = FIntPoint(1, 0);
			Cellinfo.Yaw = (Dir.Y != 0) ? 90.f : 0.f;
		}
		TargetData->CellInfos.Add(Cellinfo);
	}
	FGameplayEventData EventData;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(TargetData);
	
	AActor* ASCOwner = OwnerPawn;
	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		if (APlayerState* PS = PC->GetPlayerState<APlayerState>())
			ASCOwner = PS;
	}
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		ASCOwner, BuildPlaceEventTag, EventData);
}

void USMBuildingModeComponent::LoadBuildingDataTable()
{
	UDataTable* DataTable = LoadObject<UDataTable>(nullptr, *BuildingDataTablePath);
	if (!DataTable)
	{
		SM_LOG(this, LogSM, Error, TEXT("[BuildingModeComponent] DataTable 로드 실패"));
		return;
	}
	
	FString ContextString;
	TArray<FSMBuildingData*> AllRows;
	DataTable->GetAllRows(ContextString, AllRows);
	
	CachedBuildingData.Empty();
	for (const FSMBuildingData* Row : AllRows)
	{
		if (Row) CachedBuildingData.Add(*Row);
	}
	
	UE_LOG(LogTemp, Log, TEXT("[BuildingModeComponent] BuildingData %d개 캐싱 완료"), CachedBuildingData.Num());
}

const FSMBuildingData* USMBuildingModeComponent::GetCurrentBuildingData() const
{
	if (!CachedBuildingData.IsValidIndex(CurrentSlotIndex)) return nullptr;
	return &CachedBuildingData[CurrentSlotIndex];
}

void USMBuildingModeComponent::ServerRPC_RequestPlaceBuilding_Implementation(const TArray<FSMCellPlaceInfo>& CellInfos,
	EGridBuildingType BuildingType)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	ASMPlayerState* PS = OwnerPawn->GetPlayerState<ASMPlayerState>();
	if (!PS) return;
	
	
	
	FSMBuildPlaceTargetData* TargetData = new FSMBuildPlaceTargetData();
	TargetData->BuildingType = BuildingType;
	TargetData->CellInfos = CellInfos;
	
	FGameplayEventData EventData;
	EventData.TargetData = FGameplayAbilityTargetDataHandle(TargetData);
	

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		PS, BuildPlaceEventTag, EventData);
}

float USMBuildingModeComponent::GetCornerYawFromConnections(FIntPoint DirA, FIntPoint DirB) const
{
	if (DirA == FIntPoint(-1, 0) && DirB == FIntPoint( 0, 1)) return   0.f;  // ㄴ
	if (DirA == FIntPoint(-1, 0) && DirB == FIntPoint( 0,-1)) return 270.f;  // 역ㄴ  ← 90→270
	if (DirA == FIntPoint( 0, 1) && DirB == FIntPoint(-1, 0)) return 180.f;  // ㄱ
	if (DirA == FIntPoint( 0,-1) && DirB == FIntPoint(-1, 0)) return  90.f;  // 역ㄱ  ← 270→90

	if (DirA == FIntPoint( 0,-1) && DirB == FIntPoint( 1, 0)) return   0.f;
	if (DirA == FIntPoint( 0, 1) && DirB == FIntPoint( 1, 0)) return 270.f;  // ← 90→270
	if (DirA == FIntPoint( 1, 0) && DirB == FIntPoint( 0,-1)) return 180.f;
	if (DirA == FIntPoint( 1, 0) && DirB == FIntPoint( 0, 1)) return  90.f;  // ← 270→90
	return 0.f;
}

FSMCornerInfo USMBuildingModeComponent::GetEffectiveCornerInfo(const TArray<FIntPoint>& Path, int32 i)
{
	FSMCornerInfo Corner = GetCornerInfo(Path, i);
	if (Corner.bIsCorner) return Corner;
	
	if (Path.Num() < 2) return {false, 0.f};
	if (i != 0 && i != Path.Num()-1) return {false, 0.f};
	if (!GridManager) return {false, 0.f};
	
	FIntPoint PathDir = (i==0) ? (Path[1] - Path[0]) : (Path[Path.Num()-2] - Path.Last());
	
	FIntPoint Perps[] = {
		FIntPoint(-PathDir.Y, PathDir.X),
		FIntPoint(PathDir.Y, -PathDir.X)
	};
	
	for (const FIntPoint& PDir : Perps)
	{
		FIntPoint N = Path[i] + PDir;
		if (GridManager->IsValidGridPosition(N.X, N.Y) && !GridManager->IsCellEmpty(N.X, N.Y))
		{
			float Yaw = GetCornerYawFromConnections(FIntPoint(-PathDir.X,-PathDir.Y), PDir);
			return {true, Yaw};
		}
	}
	return {false, 0.f};
}

