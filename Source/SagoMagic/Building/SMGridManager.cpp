#include "SMGridManager.h"
#include "DrawDebugHelpers.h"
#include "Algo/Reverse.h"
#include "Net/UnrealNetwork.h"

ASMGridManager::ASMGridManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	bReplicates = true;
}

void ASMGridManager::BeginPlay()
{
	Super::BeginPlay();
	
	//이 액터의 월드 위치를 그리드 원점으로 사용
	GridOrigin = GetActorLocation();
	
	//GridData 초기화는 서버에서만 수행
	//클라이언트는 복제된 배열을 받아 사용
	if (HasAuthority())
	{
		GridData.SetNum(GridWidth * GridHeight);
	}

	if (bDrawDebugGrid)
	{
		SetActorTickEnabled(true);
	}
}

void ASMGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bDrawDebugGrid)
		DrawDebugGridLines();
}

void ASMGridManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	//GridData 전체 복제
	//PlacedActor는 복제 제외
	DOREPLIFETIME(ASMGridManager, GridData);
}

//-------------좌표 변환-------------

FIntPoint ASMGridManager::WorldToGrid(const FVector& WorldLocation) const
{
	//GridActor좌표 기준으로 셀 인덱스 계산
	//FloorToInt로 내림하여 좌하단 기준 인덱스 확보
	int32 GridX = FMath::FloorToInt((WorldLocation.X - GridOrigin.X) / CellSize);
	int32 GridY = FMath::FloorToInt((WorldLocation.Y - GridOrigin.Y) / CellSize);
	return FIntPoint(GridX, GridY);
}

FVector ASMGridManager::GridToWorld(int32 GridX, int32 GridY) const
{
	//셀 인덱스 -> 셀 중앙 월드 좌표
	float WorldX = GridOrigin.X + (GridX * CellSize) + (CellSize * 0.5f);
	float WorldY = GridOrigin.Y + (GridY * CellSize) + (CellSize * 0.5f);
	return FVector(WorldX, WorldY, GridOrigin.Z);
}

FVector ASMGridManager::SnapToGrid(const FVector& WorldLocation) const
{
	//월드 좌표 -> 셀 인덱스 -> 셀 중앙으로 스냅
	FIntPoint GridPos = WorldToGrid(WorldLocation);
	return GridToWorld(GridPos.X, GridPos.Y);
}

//-------------유효성 검사-------------

bool ASMGridManager::IsValidGridPosition(int32 GridX, int32 GridY) const
{
	return GridX >= 0 && GridX < GridWidth
		&& GridY >= 0 && GridY < GridHeight;
}

bool ASMGridManager::IsCellEmpty(int32 GridX, int32 GridY) const
{
	//범위 밖은 점유로 간주
	if (!IsValidGridPosition(GridX, GridY)) return false;
	
	return !GridData[GetCellIndex(GridX, GridY)].bIsOccupied;
}

bool ASMGridManager::CanPlaceBuilding(int32 GridX, int32 GridY, FIntPoint BuildingGridSize, int32 RotationIndex) const
{
	//회전을 반영한 실제 점유 크기 계산
	FIntPoint ActualSize = GetRotationSize(BuildingGridSize, RotationIndex);
	
	//건물이 차지하는 모든 셀을 순회하며 검사
	for (int32 DX = 0; DX < ActualSize.X; ++DX)
	{
		for (int32 DY = 0; DY < ActualSize.Y; ++DY)
		{
			//맵 경계 초과 체크
			if (!IsValidGridPosition(GridX + DX, GridY + DY)) return false;
			
			//이미 점유된 셀 체크
			if (!IsCellEmpty(GridX + DX, GridY + DY)) return false;
		}
	}
	return true;
}

//-------------배치/갱신/제거-------------

bool ASMGridManager::PlaceBuilding(int32 GridX, int32 GridY, FIntPoint BuildingGridSize, int32 RotationIndex,
	EGridBuildingType BuildingType, AActor* PlacedActor, int32 OwnerId)
{
	//서버 전용
	if (!HasAuthority()) return false;
	
	//배치 가능 여부 최종 검증
	if (!CanPlaceBuilding(GridX, GridY, BuildingGridSize, RotationIndex)) return false;
	
	FIntPoint ActualSize = GetRotationSize(BuildingGridSize, RotationIndex);
	
	//건물이 차지하는 모든 셀을 점유 처리
	for (int32 DX = 0; DX < ActualSize.X; ++DX)
	{
		for (int32 DY = 0; DY < ActualSize.Y; ++DY)
		{
			int32 Index = GetCellIndex(GridX + DX, GridY + DY);
			GridData[Index].bIsOccupied = true;
			GridData[Index].BuildingType = BuildingType;
			GridData[Index].OwnerId = OwnerId;
			GridData[Index].PlacedActor = PlacedActor;
		}
	}
	
	//배치 완료 이벤트 브로드캐스트
	OnBuildingPlaced.Broadcast(GridX, GridY, PlacedActor);
	return true;
}

FGridCell ASMGridManager::GetCellData(int32 GridX, int32 GridY) const
{
	//범위 밖이면 기본값 셀 반환
	if (!IsValidGridPosition(GridX, GridY)) return FGridCell();
	
	return GridData[GetCellIndex(GridX, GridY)];
}

AActor* ASMGridManager::GetActorAtCell(int32 GridX, int32 GridY) const
{
	if (!IsValidGridPosition(GridX, GridY)) return nullptr;
	
	return GridData[GetCellIndex(GridX, GridY)].PlacedActor.Get();
}

void ASMGridManager::UpdateActorAtCell(int32 GridX, int32 GridY, AActor* NewActor)
{
	//서버 전용
	if (!HasAuthority()) return;
	if (!IsValidGridPosition(GridX, GridX)) return;
	
	GridData[GetCellIndex(GridX, GridY)].PlacedActor = NewActor;
}

void ASMGridManager::ClearCell(int32 GridX, int32 GridY)
{
	//서버 전용
	if (!HasAuthority()) return;
	if (!IsValidGridPosition(GridX, GridX)) return;
	
	//셀 모든 필드 초기화 (bIsOccupied = false로 변경 -> 클라 동기화)
	GridData[GetCellIndex(GridX, GridY)].Clear();
}

//-------------A* 경로 탐색-------------

TArray<FIntPoint> ASMGridManager::FindPath(FIntPoint Start, FIntPoint End)
{
	//시작 == 끝이면 단일 셀 반환
	if (Start == End) return {Start};
	
	//유효하지 않은 좌표면 빈 배열 반환
	if (!IsValidGridPosition(Start.X, Start.Y) ||
		!IsValidGridPosition(End.X, End.Y))
		return {};
	
	//자료구조 초기화
	TArray<FSMAStarNode> OpenList;//탐색 후보 노드 (F값 오름차순 정렬)
	TMap<FIntPoint, bool> Closed;//처리 완료 셀 (재방문 방지)
	TMap<FIntPoint, FIntPoint> ParentMap;//경로 역추적용 부모 셀 기록
	TMap<FIntPoint, int32> GMap;//각 셀의 최적 G값 기록
	
	//시작 노드 삽입
	FSMAStarNode StartNode;
	StartNode.Grid = Start;
	StartNode.Parent = Start;
	StartNode.Direction = FIntPoint(0,0);
	StartNode.G = 0;
	StartNode.H = FMath::Abs(End.X - Start.X) + FMath::Abs(End.Y - Start.Y);
	StartNode.F = StartNode.H;
	
	OpenList.Add(StartNode);
	GMap.Add(Start, 0);
	
	//탐색 방향 : 상/하/우/좌 (대각선 없음)
	const TArray<FIntPoint> Directions = {{0,1}, {0,-1}, {1,0}, {-1,0}};
	
	//메인 탐색 루프
	while (OpenList.Num() > 0)
	{
		//F값이 가장 작은 노드 꺼내기 (동점이면 H 작은 것 우선)
		OpenList.Sort([](const FSMAStarNode& A, const FSMAStarNode& B)
		{
			if (A.F == B.F) return A.H < B.H;
			return A.F < B.F;
		});
		
		FSMAStarNode Current = OpenList[0];
		OpenList.RemoveAt(0);
		
		//이미 처리된 셀이면 스킵
		if (Closed.Contains(Current.Grid)) continue;
		
		//처리 완료 등록 + 부모 기록
		Closed.Add(Current.Grid, true);
		ParentMap.Add(Current.Grid, Current.Parent);
		
		//목표 도달 시 경로 역추적
		if (Current.Grid == End)
		{
			TArray<FIntPoint> Path;
			FIntPoint Step = End;
			
			//End -> Start 역순으로 부모를 따라가며 경로 수집
			while (Step != Start)
			{
				Path.Add(Step);
				Step = ParentMap[Step];
			}
			Path.Add(Start);
			
			//역순 배열을 Start -> End 순으로 뒤집어 반환
			Algo::Reverse(Path);
			return Path;
		}
		
		//인접 노드 탐색
		for (const FIntPoint& Dir : Directions)
		{
			FIntPoint NeighborGrid = Current.Grid + Dir;
			
			//그리드 범위 밖이면 스킵
			if (!IsValidGridPosition(NeighborGrid.X, NeighborGrid.Y)) continue;
			
			//이미 처리된 셀이면 스킵
			if (Closed.Contains(NeighborGrid)) continue;
			
			//점유된 셀은 장애물 처리 (단, 목표 셀은 통과 허용)
			if (!IsCellEmpty(NeighborGrid.X, NeighborGrid.Y) && NeighborGrid != End) continue;
			
			//방향 전환 패널티 : 직선 경롤르 우선하기 위해 꺽이면 +5
			FIntPoint NewDir = NeighborGrid - Current.Grid;
			bool bTurned = (Current.Direction != FIntPoint(0,0)) && (NewDir != Current.Direction);
			int32 TurnPenalty = bTurned ? 5 : 0;
			int32 NewG = Current.G + 10 + TurnPenalty;
			
			//이미 더 좋은 경로가 기록되어 있으면 스킵
			if (GMap.Contains(NeighborGrid) && GMap[NeighborGrid] <= NewG) continue;
			
			//새 노드 생성 및 OpenList 추가
			FSMAStarNode Neighbor;
			Neighbor.Grid = NeighborGrid;
			Neighbor.Parent = Current.Grid;
			Neighbor.Direction = NewDir;
			Neighbor.G = NewG;
			Neighbor.H = FMath::Abs(End.X - NeighborGrid.X) + FMath::Abs(End.Y - NeighborGrid.Y);
			Neighbor.F = Neighbor.G + Neighbor.H;
			
			GMap.Add(NeighborGrid, NewG);
			OpenList.Add(Neighbor);
		}
	}
	
	//경로를 찾지 못한 경우
	return {};
}

//-------------내부 헬퍼-------------

int32 ASMGridManager::GetCellIndex(int32 GridX, int32 GridY) const
{
	//2D -> 1D 인덱스 변환 : Index = Y * GridWidth + X
	return GridY * GridWidth + GridX;
}

FIntPoint ASMGridManager::GetRotationSize(FIntPoint Size, int32 RotationIndex) const
{
	//90도 또는 270도(홀수) 회전이면 X<->Y 스왑하여 실제 점유 크기 반환
	if (RotationIndex % 2 != 0) return FIntPoint(Size.Y, Size.X);
	return Size;
}

void ASMGridManager::DrawDebugGridLines() const
{
	if (GridData.Num() == 0) return;
	
	float Z = GridOrigin.Z + 1.0f;

	// 세로선 (X축 방향)
	for (int32 X = 0; X <= GridWidth; ++X)
	{
		FVector Start(GridOrigin.X + X * CellSize, GridOrigin.Y, Z);
		FVector End(GridOrigin.X + X * CellSize, GridOrigin.Y + GridHeight * CellSize, Z);
		DrawDebugLine(GetWorld(), Start, End, FColor::White, false, -1.0f, 0, 1.0f);
	}

	// 가로선 (Y축 방향)
	for (int32 Y = 0; Y <= GridHeight; ++Y)
	{
		FVector Start(GridOrigin.X, GridOrigin.Y + Y * CellSize, Z);
		FVector End(GridOrigin.X + GridWidth * CellSize, GridOrigin.Y + Y * CellSize, Z);
		DrawDebugLine(GetWorld(), Start, End, FColor::White, false, -1.0f, 0, 1.0f);
	}

	// 점유된 셀을 빨간 박스로 시각화
	for (int32 X = 0; X < GridWidth; ++X)
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			if (GridData[GetCellIndex(X, Y)].bIsOccupied)
			{
				FVector CellCenter = GridToWorld(X, Y) + FVector(0, 0, 2.0f);
				DrawDebugSolidBox(GetWorld(), CellCenter,
					FVector(CellSize * 0.45f, CellSize * 0.45f, 1.0f),
					FColor::Red, false, -1.0f, 0);
			}
		}
	}
}



