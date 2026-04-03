#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SMInteractionTargetComponent.generated.h"

class APawn;
class UMaterialInterface;
class UMeshComponent;


/**
 * 상호작용 대상 공통 컴포넌트 정의 파일
 *
 * 포함 내용:
 * - 상호작용 가능 여부
 * - 상호작용 표시 문구
 * - 상호작용 우선순위
 * - 하이라이트 대상 메시 목록
 * - 하이라이트 오버레이 머터리얼
 * - 로컬 포커스 상태
 * - 상호작용 진입 함수
 *
 * 역할:
 * - 상호작용 가능한 액터의 공통 진입점과 로컬 하이라이트 표시를 담당
 */

/** 상호작용 대상 공통 컴포넌트 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SAGOMAGIC_API USMInteractionTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMInteractionTargetComponent();

	/** BeginPlay 오버라이드 */
	virtual void BeginPlay() override;

	/** 상호작용 가능 여부 Getter */
	bool IsInteractionEnabled() const
	{
		return bInteractionEnabled;
	}

	/** 현재 로컬 포커스 여부 Getter */
	bool IsFocusedLocally() const
	{
		return bFocusedLocally;
	}

	/** 상호작용 표시 문구 Getter */
	const FText& GetInteractionDisplayText() const
	{
		return InteractionDisplayText;
	}

	/** 상호작용 우선순위 Getter */
	int32 GetInteractionPriority() const
	{
		return InteractionPriority;
	}

	/** 하이라이트 대상 메시 목록 Getter */
	const TArray<TObjectPtr<UMeshComponent>>& GetHighlightMeshComponents() const
	{
		return HighlightMeshComponents;
	}

	/** 하이라이트 오버레이 머터리얼 Getter */
	UMaterialInterface* GetHighlightOverlayMaterial() const
	{
		return HighlightOverlayMaterial;
	}

	/** 상호작용 가능 여부 Setter */
	void SetInteractionEnabled(const bool bInInteractionEnabled)
	{
		bInteractionEnabled = bInInteractionEnabled;
	}

	/** 상호작용 표시 문구 Setter */
	void SetInteractionDisplayText(const FText& InInteractionDisplayText)
	{
		InteractionDisplayText = InInteractionDisplayText;
	}

	/** 상호작용 우선순위 Setter */
	void SetInteractionPriority(const int32 InInteractionPriority)
	{
		InteractionPriority = InInteractionPriority;
	}

	/** 하이라이트 오버레이 머터리얼 Setter */
	void SetHighlightOverlayMaterial(UMaterialInterface* InHighlightOverlayMaterial)
	{
		HighlightOverlayMaterial = InHighlightOverlayMaterial;
	}

public:
	/** 상호작용 가능 여부 설정 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction Target")
	void SetInteractionEnabledRuntime(bool bInInteractionEnabled);

	/** 로컬 포커스 시작 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction Target")
	void SetFocusedLocally(bool bInFocusedLocally);

	/** 로컬 포커스 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction Target")
	void ClearLocalFocus();

	/** 상호작용 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction Target")
	void Interact(APawn* InInteractingPawn);

	/** 상호작용 가능 여부 검사 요청 */
	UFUNCTION(BlueprintCallable, Category="Interaction Target")
	bool CanInteract(APawn* InInteractingPawn) const;

protected:
	/** 오너 액터 기준 상호작용 실제 처리 */
	void HandleOwnerInteract(APawn* InInteractingPawn);

	/** 현재 하이라이트 상태 적용 */
	void ApplyHighlightState();

	/** 단일 메시 하이라이트 적용 */
	void ApplyHighlightToMesh(UMeshComponent* InMeshComponent, bool bInApplyHighlight) const;

	/** 오너 액터의 메시 자동 수집 */
	void CollectOwnerMeshComponents();

private:

public:

protected:
	/** 상호작용 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction Target")
	bool bInteractionEnabled;

	/** 상호작용 표시 문구 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction Target")
	FText InteractionDisplayText;

	/** 상호작용 우선순위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction Target")
	int32 InteractionPriority;

	/** 하이라이트 대상 메시 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction Target|Highlight")
	TArray<TObjectPtr<UMeshComponent>> HighlightMeshComponents;

	/** 하이라이트 오버레이 머터리얼 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Interaction Target|Highlight")
	TObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

	/** 메시 자동 수집 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interaction Target|Highlight")
	bool bAutoCollectOwnerMeshComponents;

private:
	/** 현재 로컬 포커스 여부 */
	bool bFocusedLocally;
};
