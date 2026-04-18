#include "SMEditModeComponent.h"

#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "SagoMagic.h"
#include "Building/SMBaseBuilding.h"
#include "Building/SMGridManager.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "Kismet/GameplayStatics.h"

USMEditModeComponent::USMEditModeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void USMEditModeComponent::SetupInputBindings()
{
	if (bInputBound) return;
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->InputComponent) return;
	
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(OwnerPawn->InputComponent);
	if (!Input) return;
	
	if (EditAction)
	{
		Input->BindAction(EditAction, ETriggerEvent::Started, this, &USMEditModeComponent::OnClick);
		Input->BindAction(EditAction, ETriggerEvent::Triggered, this, &USMEditModeComponent::OnGrab);
		Input->BindAction(EditAction, ETriggerEvent::Completed, this, &USMEditModeComponent::OnGrabEnd);
	}
	if (DeleteAction)
	{
		Input->BindAction(DeleteAction, ETriggerEvent::Started, this, &USMEditModeComponent::OnDeleteSelected);
	}
	bInputBound = true;
}

void USMEditModeComponent::EnableEditMode()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;
	
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = 
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!Subsystem || !EditIMC) return;
	
	Subsystem->AddMappingContext(EditIMC, 1);
	bIsEditMode = true;
	SetComponentTickEnabled(true);
	
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
	
	FEditModeMsg Msg;
	Msg.bIsActive = true;
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(SMUITag::Event_EditMode, Msg);
}

void USMEditModeComponent::DisableEditMode()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;
	
	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return;
	
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) return;
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem && EditIMC)
	{
		Subsystem->RemoveMappingContext(EditIMC);
	}

	if (CurrentIntent == EClickIntent::Grabbing && GridManager)
	{
		RestoreToOriginalPositions();
	}
	
	bIsEditMode = false;
	CurrentIntent = EClickIntent::None;
	bLastDeltaValid = false;
	SetComponentTickEnabled(false);
	ClearSelection();
	
	FEditModeMsg Msg;
	Msg.bIsActive = false;
	UGameplayMessageSubsystem::Get(GetWorld()).BroadcastMessage(SMUITag::Event_EditMode, Msg);
}


void USMEditModeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GridManager)
	{
		GridManager = Cast<ASMGridManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ASMGridManager::StaticClass())
		);
	}
}

void USMEditModeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
									   FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateGrabbing();
}

void USMEditModeComponent::OnClick(const FInputActionValue& Value)
{
	if (CurrentIntent == EClickIntent::Grabbing) return;
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !GridManager) return;
	// 스크린 좌표 기록
	float MX = 0.f, MY = 0.f;
	if (!PC->GetMousePosition(MX, MY))
	{
		SM_LOG(this, LogSM, Warning, TEXT("GetMousePosition 실패"));
		return;
	}
	PressScreenPos = FVector2D(MX, MY);
	bDragConfirmed = false;
	// 월드 좌표 기록
	FHitResult Hit;
	if (!PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		SM_LOG(this, LogSM, Warning, TEXT("GetHitResultUnderCursor 실패"));
		return;
	}
	PressWorldPos = Hit.Location;

	// 누른 위치가 이미 선택된 펜스?
	ASMBaseBuilding* HitBuilding = Cast<ASMBaseBuilding>(Hit.GetActor());
	if (HitBuilding && SelectionSnapshot.Contains(HitBuilding))
	{
		CurrentIntent = EClickIntent::Grabbing;
		bIsCurrentPosValid = false;
		FVector GroundPos;
		if (GetWorldPosUnderCursor(GroundPos))
			GrabAnchorGrid = GridManager->WorldToGrid(GroundPos);
		else
			GrabAnchorGrid = GridManager->WorldToGrid(Hit.Location);
		
		bClickWasGrabTransition = true;
		
		TArray<ASMBaseBuilding*> ToGrab;
		for (auto& [B, _] : SelectionSnapshot)
		{
			if (B)
				ToGrab.Add(B);
		}
		ServerRPC_SetGrabState(ToGrab, true);
		SM_LOG(this, LogSM, Error, TEXT("선택 -> 움직이기"));
	}
	else
	{
		CurrentIntent = EClickIntent::BoxSelect;
		ClearSelection();
		SM_LOG(this, LogSM, Error, TEXT("선택 -> 삭제"));
	}
}

void USMEditModeComponent::OnGrab(const FInputActionValue& Value)
{
	if (CurrentIntent != EClickIntent::BoxSelect) return;
	
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !GridManager) return;
	
	float MX, MY;
	if (!PC->GetMousePosition(MX, MY))
	{
		return;
	}
	FVector2D CurrentScreen(MX, MY);
	
	if (!bDragConfirmed &&
		FVector2D::Distance(CurrentScreen, PressScreenPos) >= DragThreshold)
	{
		bDragConfirmed = true;
	}
	if (!bDragConfirmed) return;
	
	for (ASMBaseBuilding* B : SelectedActors)
		ApplyHighlight(B, false);
	
	SelectedActors.Empty();
	SelectionSnapshot.Empty();
	SelectActorsInScreenBox(PressScreenPos, CurrentScreen);
}

void USMEditModeComponent::OnGrabEnd(const FInputActionValue& Value)
{
	if (CurrentIntent == EClickIntent::Grabbing)
	{
		if (bClickWasGrabTransition)
		{
			bClickWasGrabTransition = false;
			bDragConfirmed = false;
			return;
		}
		if (!bIsCurrentPosValid)
		{
			RestoreToOriginalPositions();
			for (auto& [Building, _] : SelectionSnapshot)
				ApplyHighlight(Building, true);
			CurrentIntent = EClickIntent::None;
			bDragConfirmed = false;
			bLastDeltaValid = false;
			return;
		}
		FVector CursorWorld;
		bool bCanPlace = GetWorldPosUnderCursor(CursorWorld);
		
		if (bCanPlace)
		{
			FIntPoint CurrentGrid = GridManager->WorldToGrid(CursorWorld);
			FIntPoint Delta = CurrentGrid - GrabAnchorGrid;
			
			TSet<FIntPoint> OldGridSet;
			for (auto& [B, OrigGrid] : SelectionSnapshot)
				OldGridSet.Add(OrigGrid);
			
			TArray<ASMBaseBuilding*> Actors;
			TArray<FIntPoint> OldGrids;
			TArray<FIntPoint> NewGrids;
			bool bAllValid = true;

			for (auto& [Building, OrigGrid] : SelectionSnapshot)
			{
				if (!Building)
				{
					bAllValid = false;
					break;
				}
				FIntPoint NewGrid = OrigGrid + Delta;
				
				if (!GridManager->IsValidGridPosition(NewGrid.X, NewGrid.Y))
				{
					bAllValid = false;
					break;
				}
				if (!GridManager->IsCellEmpty(NewGrid.X, NewGrid.Y) && !OldGridSet.Contains(NewGrid))
				{
					bAllValid = false;
					break;
				}
				Actors.Add(Building);
				OldGrids.Add(OrigGrid);
				NewGrids.Add(NewGrid);
			}

			if (bAllValid)
			{
				SM_LOG(this, LogSM, Error, TEXT("배치 확정 ServerRPC 호출"));
				ServerRPC_MoveBuildings(Actors, OldGrids, NewGrids);
				for (int32 i = 0 ; i < Actors.Num(); i++)
					SelectionSnapshot[Actors[i]] = NewGrids[i];
			}
			else
			{
				SM_LOG(this, LogSM, Error, TEXT("배치 불가"));
				RestoreToOriginalPositions();
				for (auto& [Building, _] : SelectionSnapshot)
					ApplyHighlight(Building, true);
			}
		}
		else
		{
			SM_LOG(this, LogSM, Error, TEXT("커서 위치 없음, 원위치"));
			RestoreToOriginalPositions();
		}
		CurrentIntent = EClickIntent::None;
		bDragConfirmed = false;
		bLastDeltaValid = false;
		return;
	}

	if (!bDragConfirmed)
	{
		ClearSelection();
		SM_LOG(this, LogSM, Error, TEXT("단순 클릭"));
	}
	CurrentIntent = EClickIntent::None;
	bDragConfirmed = false;
}

void USMEditModeComponent::OnDeleteSelected(const FInputActionValue& Value)
{
	if (SelectedActors.IsEmpty()) return;
	SM_LOG(this, LogSM, Error, TEXT("25"));
	TArray<ASMBaseBuilding*> ToDelete;
	for (ASMBaseBuilding* B : SelectedActors)
	{
		if (B)
			ToDelete.Add(B);
	}
	ServerRPC_DeleteBuildings(ToDelete);
	ClearSelection();
}

void USMEditModeComponent::SelectActorsInScreenBox(const FVector2D& ScreenA, const FVector2D& ScreenB)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !GridManager) return;
	FVector2D BoxMin(FMath::Min(ScreenA.X, ScreenB.X), FMath::Min(ScreenA.Y, ScreenB.Y));
	FVector2D BoxMax(FMath::Max(ScreenA.X, ScreenB.X), FMath::Max(ScreenA.Y, ScreenB.Y));
	
	for (TActorIterator<ASMBaseBuilding> It(GetWorld()); It; ++It)
	{
		ASMBaseBuilding* Building = *It;
		if (!Building) continue;
		
		FVector2D ScreenPos;
		if (!PC->ProjectWorldLocationToScreen(Building->GetActorLocation(), ScreenPos))
			continue;
		if (ScreenPos.X >= BoxMin.X && ScreenPos.X <= BoxMax.X &&
			ScreenPos.Y >= BoxMin.Y && ScreenPos.Y <= BoxMax.Y)
		{
			SelectedActors.AddUnique(Building);
			SelectionSnapshot.Add(Building,
				GridManager->WorldToGrid(Building->GetActorLocation()));
			ApplyHighlight(Building, true);
		}
	}
}

void USMEditModeComponent::ApplyHighlight(ASMBaseBuilding* Building, bool bOn)
{
	if (!Building || !HighLightMaterial) return;
	TArray<UMeshComponent*> Meshes;
	Building->GetComponents<UMeshComponent>(Meshes);
	if (bOn)
	{
		TArray<TObjectPtr<UMaterialInterface>> Originals;
		for (UMeshComponent* Mesh : Meshes)
		{
			for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
			{
				Originals.Add(Mesh->GetMaterial(i));
				Mesh->SetMaterial(i, HighLightMaterial);
			}
		}
		OriginalMaterials.Add(Building,Originals);
 	}
	else
	{
		TArray<TObjectPtr<UMaterialInterface>>* Originals = OriginalMaterials.Find(Building);
		if (!Originals) return;
		int32 SlotIdx =  0;
		for (UMeshComponent* Mesh : Meshes)
		{
			for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
			{
				if (Originals->IsValidIndex(SlotIdx))
					Mesh->SetMaterial(i, (*Originals)[SlotIdx]);
				++SlotIdx;
			}
		}
		OriginalMaterials.Remove(Building);
	}
}

void USMEditModeComponent::ClearSelection()
{
	for (ASMBaseBuilding* B : SelectedActors)
		ApplyHighlight(B, false);
	SelectedActors.Empty();
	SelectionSnapshot.Empty();
	OriginalMaterials.Empty();
}

bool USMEditModeComponent::GetWorldPosUnderCursor(FVector& OutPos) const
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !GridManager) return false;
	
	FVector WorldLocation, WorldDirection;
	if (!PC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection)) return false;
	
	FCollisionQueryParams Params;
	for (auto& [Building, Grid] : SelectionSnapshot)
	{
		if (Building) Params.AddIgnoredActor(Building);
	}
	
	FHitResult Hit;
	FVector TraceEnd = WorldLocation + WorldDirection * 10000.f;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, WorldLocation, TraceEnd, ECC_Visibility, Params))
		return false;
	
	OutPos = Hit.Location;
	return true;
}

void USMEditModeComponent::UpdateGrabbing()
{
	if (CurrentIntent != EClickIntent::Grabbing) return;
	if (!GridManager) return;
	
	FVector CursorWorld;
	if (!GetWorldPosUnderCursor(CursorWorld)) return;
	
	FIntPoint Delta = GridManager->WorldToGrid(CursorWorld) - GrabAnchorGrid;
	
	TSet<FIntPoint> OldGridSet;
	for (auto& [B, OrigGrid] : SelectionSnapshot)
		OldGridSet.Add(OrigGrid);
	
	bool bAllValid = true;
	TArray<ASMBaseBuilding*> Actors;
	TArray<FVector> Locations;
	
	for (auto& [Building, OrigGrid] : SelectionSnapshot)
	{
		if (!Building)
		{
			bAllValid = false;
			continue;
		}
		
		FIntPoint NewGrid = OrigGrid + Delta;
		FVector NewWorld = GridManager->GridToWorld(NewGrid.X, NewGrid.Y);
		NewWorld.Z = CursorWorld.Z + 3.f;
		
		Building->SetActorLocation(NewWorld);
		Actors.Add(Building);
		Locations.Add(NewWorld);
		
		if (!GridManager->IsValidGridPosition(NewGrid.X, NewGrid.Y))
			bAllValid = false;
		else if (!GridManager->IsCellEmpty(NewGrid.X, NewGrid.Y) && !OldGridSet.Contains(NewGrid))
			bAllValid = false;
	}
	if (bAllValid != bIsCurrentPosValid)
	{
		bIsCurrentPosValid = bAllValid;
		UMaterialInterface* Mat = bAllValid ? ValidMaterial : InValidMaterial;
		if (Mat)
		{
			for (ASMBaseBuilding* Building : Actors)
			{
				TArray<UMeshComponent*> Meshes;
				Building->GetComponents<UMeshComponent>(Meshes);
				for (UMeshComponent* Mesh : Meshes)
					for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
						Mesh->SetMaterial(i, Mat);
			}
		}
	}
	
	if (!bLastDeltaValid || Delta != LastPreviewDelta)
	{
		LastPreviewDelta = Delta;
		bLastDeltaValid = true;
		ServerRPC_PreviewMove(Actors, Locations);
	}
}

void USMEditModeComponent::RestoreToOriginalPositions()
{
	if (!GridManager) return;
	
	TArray<ASMBaseBuilding*> Actors;
	for (auto& [Building, OrigGrid] : SelectionSnapshot)
	{
		if (!Building) continue;
		FVector RestorePos = GridManager->GridToWorldWithHeight(OrigGrid.X, OrigGrid.Y);
		Building->SetActorLocation(RestorePos);
		Actors.Add(Building);
	}
	if (!Actors.IsEmpty())
		ServerRPC_SetGrabState(Actors, false);
}

void USMEditModeComponent::MulticastRPC_SetBuildCollision_Implementation(const TArray<ASMBaseBuilding*>& Actors,
                                                                         bool bGrabbing)
{
	for (ASMBaseBuilding* Building : Actors)
	{
		if (!Building) continue;
		TArray<UPrimitiveComponent*> Prims;
		Building->GetComponents<UPrimitiveComponent>(Prims);
		Building->SetIsBeingMoved(bGrabbing);
		for (UPrimitiveComponent* Prim : Prims)
		{
			Prim->SetCollisionResponseToChannel(
				ECC_Pawn, bGrabbing ? ECR_Ignore : ECR_Block);
		}
	}
}

void USMEditModeComponent::ServerRPC_MoveBuildings_Implementation(const TArray<ASMBaseBuilding*>& Actors,
                                                                  const TArray<FIntPoint>& OldGrids, const TArray<FIntPoint>& NewGrids)
{
	if (!GridManager || Actors.Num() != NewGrids.Num() 
		|| Actors.Num() != OldGrids.Num())
		return;
	//이동 전 셀 데이터 저장 (건물 타입, 소유자 정보 보존)
	TArray<FGridCell> OldCellData;
	for (int32 i = 0; i <OldGrids.Num(); ++i)
	{
		OldCellData.Add(GridManager->GetCellData(OldGrids[i].X, OldGrids[i].Y));
	}
	
	//모든 건물의 기존 셀 점유 해제
	for (ASMBaseBuilding* Building : Actors)
	{
		if (Building)
			GridManager->ClearCellsByActor(Building);
	}
	
	//1단계 : 전체 유효성 검사
	bool bAllServerValid = true;
	for (int32 i = 0; i < Actors.Num(); ++i)
	{
		if (!Actors[i])
		{
			bAllServerValid = false;
			break;
		}
		FIntPoint NewGrid = NewGrids[i];
		if (!GridManager->IsValidGridPosition(NewGrid.X, NewGrid.Y)||
			!GridManager->IsCellEmpty(NewGrid.X, NewGrid.Y) ||
			!GridManager->HasHeightCache(NewGrid.X, NewGrid.Y))
		{
			bAllServerValid = false;
			break;
		}
	}
	
	USMSyncDataManager* DataManager = USMSyncDataManager::Get(this);
	TArray<FIntPoint> GridSizes;
	for (int32 i = 0; i < OldCellData.Num(); ++i)
	{
		FIntPoint Size(1, 1);
		if (DataManager)
		{
			if (const FSMBuildingData* BuildData = DataManager->GetBuildData(OldCellData[i].BuildingType))
				Size = BuildData->GridSize;
		}
		GridSizes.Add(Size);
	}
	
	//2단계 : 전부 유효 -> 새 위치 배치 / 하나라도 실패 -> 전부 원위치
	TArray<FVector> FinalLocations;
	if (bAllServerValid)
	{
		for (int32 i = 0; i < Actors.Num(); ++i)
		{
			if (!Actors[i]) continue;
			FVector NewWorld = GridManager->GridToWorldWithHeight(NewGrids[i].X, NewGrids[i].Y);
			FinalLocations.Add(NewWorld);
			GridManager->PlaceBuilding(NewGrids[i].X, NewGrids[i].Y, GridSizes[i], 0,
				OldCellData[i].BuildingType, Actors[i], OldCellData[i].OwnerId);
		}
	}
	else
	{
		for (int32 i = 0; i < Actors.Num(); ++i)
		{
			if (!Actors[i]) continue;
			FVector OldWorld = GridManager->GridToWorldWithHeight(OldGrids[i].X, OldGrids[i].Y);
			FinalLocations.Add(OldWorld);
			GridManager->PlaceBuilding(OldGrids[i].X, OldGrids[i].Y, GridSizes[i], 0,
				OldCellData[i].BuildingType, Actors[i], OldCellData[i].OwnerId);
		}
	}
	//모든 클라이언트에 최종 위치 전파 + 충돌 복원
	MulticastRPC_FinalizeMove(Actors, FinalLocations);
}

void USMEditModeComponent::MulticastRPC_FinalizeMove_Implementation(const TArray<ASMBaseBuilding*>& Actors,
	const TArray<FVector>& FinalLocations)
{
	for (int32 i = 0; i < Actors.Num(); ++i)
	{
		if (!Actors[i]) continue;
		
		Actors[i]->SetActorLocation(FinalLocations[i]);
		Actors[i]->SetIsBeingMoved(false);
		
		//배치 완료 후 Pawn 충돌 복원
		TArray<UPrimitiveComponent*> Prims;
		Actors[i]->GetComponents<UPrimitiveComponent>(Prims);
		for (UPrimitiveComponent* Prim : Prims)
		{
			Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		}
	}
}

void USMEditModeComponent::ServerRPC_PreviewMove_Implementation(const TArray<ASMBaseBuilding*>& Actors,
                                                                const TArray<FVector>& Locations)
{
	MulticastRPC_PreviewMove(Actors, Locations);
}

void USMEditModeComponent::MulticastRPC_PreviewMove_Implementation(const TArray<ASMBaseBuilding*>& Actors,
	const TArray<FVector>& Locations)
{
	//조작 중인 클라이언트는 UpdateGrabbing에서 이미 처리하므로 스킵
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn && OwnerPawn->IsLocallyControlled()) return;
	
	for (int32 i = 0; i < Actors.Num(); i++)
	{
		if (!Actors[i])
			continue;
		Actors[i]->SetActorLocation(Locations[i]);
	}
}

void USMEditModeComponent::ServerRPC_SetGrabState_Implementation(const TArray<ASMBaseBuilding*>& Actors, bool bGrabbing)
{
	MulticastRPC_SetBuildCollision(Actors, bGrabbing);
}

void USMEditModeComponent::ServerRPC_DeleteBuildings_Implementation(const TArray<ASMBaseBuilding*>& Actors)
{
	if (!GridManager) return;
	for (ASMBaseBuilding* Building : Actors)
	{
		if (!Building) continue;
		GridManager->ClearCellsByActor(Building);
		Building->Destroy();
	}
}
