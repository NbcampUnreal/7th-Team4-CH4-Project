#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FGridCell.h"
#include "FSMAStarNode.h"
#include "FSMSelectActorInfo.h"
#include "SMGridManager.generated.h"

/**
 * 건물 배치 완료 시 브로드캐스되는 델리게이트
 * @param GridX			배치된 셀의 X 인덱스
 * @param GridY			배치된 셀의 Y 인덱스
 * @param PlacedActor	실제 스폰된 건물 액터
 */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnSMBuildingPlaced, int32, int32, AActor*);

/**
 * ASMGridManager - 멀티플레이 그리드 배치 시스템의 중식 액터
 * 
 * [역할]
 * - 레벨에 1개만 배치되며, 전체 셀 점유 상태를 2D 배열(1D 저장)로 관리
 * - 자표 변환, 유효성 검사, 배치/제거, 경로 탐색 담당
 * 
 * [네트워크]
 *  - 서버(Authority) : 배치/제거/갱신 등 상태 변경 권한 보유
 *  - 클라이언트 : GridData를 복제 받아 IsCellEmpty()로 고스트 프리뷰 유효성만 체크
 *  - 건물 액터는 서버 SpawnActor + bReplicates = true로 클라이언트에 자동 복제
 *  - GridData[].PlacedActor는 복제 제외 (서버 전용 참조)
 */
UCLASS()
class SAGOMAGIC_API ASMGridManager : public AActor
{
	GENERATED_BODY()

public:
	ASMGridManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	/** GridData 복제 등록 */
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	// 좌표 변환 (클라이언트/서버 모두 사용 가능)
	
	/**
	 * 월드 좌표 -> 그리드 셀 인덱스(X,Y) 변환
	 * FloorToInt로 내림하여 해당 셀의 좌하단 기준 인덱스를 반환
	 * @param WorldLocation 변환할 월드 좌표
	 * @return				그리드 셀 인덱스 (FIntPoint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FIntPoint WorldToGrid(const FVector& WorldLocation) const;

	/**
	 * 그리드 셀 인덱스 -> 셀 중앙의 월드 좌표 변환
	 * Z는 GridOrigin.Z 고정
	 * @param GridX 셀 X 인덱스
	 * @param GridY 셀 Y 인덱스
	 * @return		셀 중앙의 월드 좌표
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GridToWorld(int32 GridX, int32 GridY) const;
	
	/**
	 * 월드 좌표를 가장 가까운 셀 중앙으로 스냅
	 * WorldToGrid -> GridToWorld를 연속 호출하는 편의 함수
	 * @param WorldLocation	변환할 원드 좌표
	 * @return				그리드 셀 인덱스 (FIntPoint)
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector SnapToGrid(const FVector& WorldLocation) const;
	
	// 유효성 검사 (클라/서버 모두 사용 가능)

	/**
	 * 그리드 좌표가 배열 범위 내인지 확인
	 * 모든 셀 접근 반드시 호출하여 범위 초과를 방지
	 * @param GridX 검사할 셀 X 인덱스
	 * @param GridY 검사한 셀 Y 인덱스
	 * @return		유효하면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsValidGridPosition(int32 GridX, int32 GridY) const;

	/**
	 * 해당 셀이 비어있는지 확인
	 * 클라이언트에서 고스트 프리뷰 유효성 체크에 주로 사용
	 * 범위 밖이면 false 반환 (점유로 간주)
	 * @param GridX 검사할 셀 X 인덱스
	 * @param GridY 검사할 셀 Y 인덱스
	 * @return		비어있으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool IsCellEmpty(int32 GridX, int32 GridY) const;

	/**
	 * 지정 위치에 건물을 배치할 수 있는지 검사
	 * 회전을 반영한 실제 크기(GridSize)로 범위 + 점유 여부를 모두 확인
	 * @param GridX				좌하단 기준 X 인덱스
	 * @param GridY				좌하단 기준 Y 인덱스
	 * @param BuildingGridSize	건물의 기본 그리드 크기 (회전 전)
	 * @param RotationIndex		회전 단계 (0부터 3까지 0도, 90도, 180도, 270도)
	 * @return					모든 셀이 유효하고 비어있으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool CanPlaceBuilding(int32 GridX, int32 GridY, FIntPoint BuildingGridSize, int32 RotationIndex = 0) const;
	
	// 배치/갱신/제거 (서버 전용)

	/**
	 * 건물을 그리드에 등록
	 * 서버 전용, 클라이언트 호출 시 false 반환
	 * CanPlaceBuilding() 통과 후 해당 셀들을 점유 처리하고
	 * OnBuildingPlaced 델리게이트를 브로드캐스트함
	 * GridData 변경은 DOREPLIFETIME으로 클라이언트에 자동 동기화됨
	 * @param GridX				좌하단 기준 X 인덱스
	 * @param GridY				좌하단 기준 Y 인덱스
	 * @param BuildingGridSize	건물의 기본 그리드 크기
	 * @param RotationIndex		회전 단계
	 * @param BuildingType		건물 타입
	 * @param PlacedActor		실제 스폰된 건물 액터
	 * @param OwnerId			시공자 플레이어 ID
	 * @return					배치 성공 시 true
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool PlaceBuilding(int32 GridX, int32 GridY, FIntPoint BuildingGridSize, int32 RotationIndex,
					   EGridBuildingType BuildingType, AActor* PlacedActor, int32 OwnerId);

	/**
	 * 셀 데이터 반환
	 * 클라이언트에서 호출 시 항상 nullptr
	 * 범위 밖이면 기본값 반환
	 * @param GridX	조회할 셀 X 인덱스
	 * @param GridY	조회할 셀 Y 인덱스
	 * @return		해당 셀의 FGridCell 복사본
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FGridCell GetCellData(int32 GridX, int32 GridY) const;

	/**
	 * 셀에 배치된 액터 반환
	 * 서버 전용 PlacedActor는 복제 제외
	 * @param GridX 조회할 셀 X 인덱스
	 * @param GridY 조회할 셀 Y 인덱스
	 * @return		배치된 액터, 없으면 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	AActor* GetActorAtCell(int32 GridX, int32 GridY) const;

	/**
	 * 셀의 PlacedActor 참조만 교체 (점유 상태/타입은 유지)
	 * 서버 전용
	 * 펜스 코너 기둥 교체 시 기존 직선 액터를 코너 기둥으로 갱신할 때 쓰임
	 * @param GridX		교체할 셀 X 인덱스
	 * @param GridY		교체할 셀 Y 인덱스
	 * @param NewActor	새로 등록할 액터
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void UpdateActorAtCell(int32 GridX, int32 GridY, AActor* NewActor);

	/**
	 * 셀 데이터 전체 초기화
	 * 서버 전용
	 * 건물 삭제 시 호출
	 * GridData 변경은 DOREPLIFETIME으로 클라이언트에 자동 동기화됨
	 * @param GridX	초기화할 셀 X 인덱스
	 * @param GridY 초기화할 셀 Y 인덱스
	 */
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ClearCell(int32 GridX, int32 GridY);

	/**
	 * A* 알고리즘으로 시작점 ~ 끝점 간 최단 경로 탐색
	 * 서버 권장으로 펜스 직선 배치 경로 계산에 사용
	 * 클라이언트에서도 호출 가능하나, 실제 배치는 서버에서만 처리
	 * - 점유된 셀은 장애물 처리(시작/끝점 제외)
	 * - 방향 전환 시 패널티 부여하여 직선 경로 우선
	 * - 시작 == 끝이면 단일 배열 반환
	 * - 경로 없으면 빈 배열 반환
	 * @param Start 탐색 시작 셀 인덱스
	 * @param End	탐색 목표 셀 인덱스
	 * @return		시작~끝을 잇는 셀 인덱스 배열 (순서대로)
	 */
	TArray<FIntPoint> FindPath(FIntPoint Start, FIntPoint End);
public:
	// 그리드 설정
	
	/** 셀 하나의 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	float CellSize = 100.0f;
	
	/** 그리드 가로 셀 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridWidth = 50;
	
	/** 그리드 세로 셀 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridHeight = 50;
	
	/**
	 * 그리드 원점
	 * BeginPlay 시 이 액터의 위치로 자동 설정됨
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;
	
	/**
	 * 그리드 셀 배열 - 서버에서 관리, 클라이언트에 복제
	 * 저장 방식 : Index = Y * GridWidth + X (1D)
	 * 복제 항목 : bIsOccupied / BuildingType / OwnerId
	 * 복제 제외 : PlacedActor
	 * 클라이언트는 이 배열을 읽기 전용으로 사용하여
	 * IsCellEmpty()로 고스트 프리뷰 유효성을 체크함
	 */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<FGridCell> GridData;
	
	/**
	 * 건물 배치 완료 이벤트
	 * PlacedBuilding() 성공 시 서버에서 브로드캐스트
	 * GameMode, WaveManager 등에서 구독하여 후처리에 활용 가능
	 */
	FOnSMBuildingPlaced OnBuildingPlaced;
	
	/** 디버그용 그리드 라인 및 점유 셀 시각화 */
	UPROPERTY(EditAnywhere, Category = "Grid|Debug")
	bool bDrawDebugGrid = false;
private:
	/**
	 * 그리드 좌표 -> 1D 배열 인덱스 변환
	 * Index = Y * GridWidget + X
	 */
	int32 GetCellIndex(int32 GridX, int32 GridY) const;
	
	/**
	 * 회전을 반영한 실제 건물 크기 반환
	 * RotationIndex가 홀수(90도, 270도)이면 X<->Y 스왑
	 * @param size			회전 전 기본 크기
	 * @param RotationIndex 회전 단계
	 * @return				회전 적용된 실제 크기
	 */
	FIntPoint GetRotationSize(FIntPoint Size, int32 RotationIndex) const;
	
	/** Tick에서 호출되는 디버그 그리드 라인 드로우 */
	void DrawDebugGridLines() const;
};
