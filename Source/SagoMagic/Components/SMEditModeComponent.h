#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SMEditModeComponent.generated.h"

class ASMGridManager;
class ASMBaseBuilding;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * EditMode에서 마우스 클릭의 의도를 추적하는 상태
 * 
 * None : 기본 상태
 * BoxSelect : 마우스 드래그로 다수 건물 선택 중
 * Grabbing : 선택된 건물을 마우스로 끌어 이동 중 (토글 방식 - 클릭으로 진입/확정)
 */
UENUM(BlueprintType)
enum class EClickIntent : uint8
{
	None,
	BoxSelect,
	Grabbing
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMEditModeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USMEditModeComponent();
	
	void SetupInputBindings();
	void EnableEditMode();
	void DisableEditMode();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
private:
	//입력 핸들러
	void OnClick(const FInputActionValue& Value);
	void OnGrab(const FInputActionValue& Value);// BoxSelect 드래그 중 선택 갱신
	void OnGrabEnd(const FInputActionValue& Value);// 버튼을 뗄 때 확정 처리
	void OnDeleteSelected(const FInputActionValue& Value);
	
	//내부 헬퍼
	void SelectActorsInScreenBox(const FVector2D& ScreenA, const FVector2D& ScreenB);
	void ApplyHighlight(ASMBaseBuilding* Building, bool bOn);
	void ClearSelection();
	bool GetWorldPosUnderCursor(FVector& OutPos) const;
	
	/** Tick에서 호출 : Grabbing 중 건물을 마우스 위치에 맞춰 이동 + 서버에 Preview 전송 */
	void UpdateGrabbing();
	
	/** Grabbing 취소 시 SelectionSnapshot 기준으로 건물 원위치 복원 + 충돌 재활성화 */
	void RestoreToOriginalPositions();
	
	//RPC
	/** 배치 학정 : 서버에서 원자적으로 전체 이동 or 전체 원위치 처리 */
	UFUNCTION(Server, Reliable)
	void ServerRPC_MoveBuildings(
		const TArray<ASMBaseBuilding*>& Actors,
		const TArray<FIntPoint>& OldGrids,
		const TArray<FIntPoint>& NewGrids);
	
	/** 선택된 Building 삭제 */
	UFUNCTION(Server, Reliable)
	void ServerRPC_DeleteBuildings(
		const TArray<ASMBaseBuilding*>& Actors);
	
	/** Grab 미리보기 : 그리드 셀이 변경될 때만 전송 */
	UFUNCTION(Server, Unreliable)
	void ServerRPC_PreviewMove(
		const TArray<ASMBaseBuilding*>& Actors,
		const TArray<FVector>& Locations);
	
	/** 조작 중인 클라를 제외한 모든 클라이언트에서 건물 위치 갱신 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_PreviewMove(
		const TArray<ASMBaseBuilding*>& Actors,
		const TArray<FVector>& Locations);
	
	/** Grab 시작/종료 : 모든 클라이언트에서 Pawn 충돌 채널 동기 변경 */
	UFUNCTION(Server, Reliable)
	void ServerRPC_SetGrabState(
		const TArray<ASMBaseBuilding*>& Actors,
		bool bGrabbing);
	
	/** 배치 확정 후 모든 클라이언트에서 최종 위치 설정 + 충돌 복원 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_FinalizeMove(
		const TArray<ASMBaseBuilding*>& Actors,
		const TArray<FVector>& FinalLocations
	);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_SetBuildCollision(
		const TArray<ASMBaseBuilding*>& Actors,
		bool bGrabbing);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> EditIMC;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> EditAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DeleteAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	int32 MaxBuildDistance = 1500.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Highlight")
	TObjectPtr<UMaterialInterface> HighLightMaterial;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsEditMode = false;
	
private:
	/** 현재 선택된 건물 목록 - 하이라이트 해제, 삭제 순회에 사용 */
	UPROPERTY()
	TArray<TObjectPtr<ASMBaseBuilding>> SelectedActors;
	
	UPROPERTY()
	TObjectPtr<ASMGridManager> GridManager = nullptr;
	
	bool bInputBound = false;
	
	EClickIntent CurrentIntent = EClickIntent::None;
	
	FVector2D PressScreenPos;// 누른 순간 스크린 좌표
	FVector PressWorldPos;// 누른 순간 월드 좌표
	bool bDragConfirmed = false;// DragThreshold 넘었는가
	float DragThreshold = 8.f;
	
	FIntPoint GrabAnchorGrid;//이동 기준 그리드

	/**
	 * 선택된 건물의 원본 그리드 좌표 맵
	 * - Grabbing 중 이동 델타 계산 : NewGrid = OrigGrid + Delta
	 * - 배치 취소 시 원위치 복원 기준
	 * - 배치 성공 시 NewGrid로 갱신
	 */
	TMap<TObjectPtr<ASMBaseBuilding>, FIntPoint> SelectionSnapshot;
	/** 하이라이트 적용 전 원본 머테리얼 저장 - 선택 해제 시 복원 */
	TMap<TObjectPtr<ASMBaseBuilding>, TArray<TObjectPtr<UMaterialInterface>>> OriginalMaterials;

	/**
	 * true = 이번 클릭이 "선택 -> Grabbing" 진입 클릭
	 * false = 이번 클릭이 배치 확정 클릭
	 * OnGrabEnd에서 두 번의 클릭을 구분하는 데 사용
	 */
	bool bClickWasGrabTransition = false;
	
	/** 마지막으로 서버에 전송한 Preview 델타. 중복 전송 방지 */
	FIntPoint LastPreviewDelta = FIntPoint(0, 0);
	bool bLastDeltaValid = false;
};
