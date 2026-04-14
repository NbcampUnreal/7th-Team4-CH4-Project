#pragma once

#include "CoreMinimal.h"
#include "Inventory/Items/Fragments/SMItemFragment.h"
#include "SMWorldVisualFragment.generated.h"

class UStaticMesh;
class UMaterialInterface;


/**
 * 월드 비주얼 Fragment 정의 파일
 *
 * 포함 내용:
 * - 월드 드랍 메시
 * - 오버라이드 머티리얼
 * - 월드 스케일
 *
 * 역할:
 * - 월드 드랍 액터 표시용 비주얼 정보 제공
 */

/** 월드 비주얼 Fragment */
UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class SAGOMAGIC_API USMWorldVisualFragment : public USMItemFragment
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMWorldVisualFragment()
		: WorldScale(FVector(1.0f, 1.0f, 1.0f))
	{
	}

	/** 월드 메시 Getter */
	const TSoftObjectPtr<UStaticMesh>& GetWorldMesh() const
	{
		return WorldMesh;
	}

	/** 오버라이드 머티리얼 Getter */
	const TSoftObjectPtr<UMaterialInterface>& GetOverrideMaterial() const
	{
		return OverrideMaterial;
	}

	/** 컬러 슬롯 머티리얼 Getter */
	const TSoftObjectPtr<UMaterialInterface>& GetColorSlotMaterial() const
	{
		return ColorSlotMaterial;
	}

	/** 월드 스케일 Getter */
	const FVector& GetWorldScale() const
	{
		return WorldScale;
	}

	/** 월드 메시 Setter */
	void SetWorldMesh(const TSoftObjectPtr<UStaticMesh>& InWorldMesh)
	{
		WorldMesh = InWorldMesh;
	}

	/** 오버라이드 머티리얼 Setter */
	void SetOverrideMaterial(const TSoftObjectPtr<UMaterialInterface>& InOverrideMaterial)
	{
		OverrideMaterial = InOverrideMaterial;
	}

	/** 컬러 슬롯 머티리얼 Setter */
	void SetColorSlotMaterial(const TSoftObjectPtr<UMaterialInterface>& InColorSlotMaterial)
	{
		ColorSlotMaterial = InColorSlotMaterial;
	}

	/** 월드 스케일 Setter */
	void SetWorldScale(const FVector& InWorldScale)
	{
		WorldScale = InWorldScale;
	}

public:
	/** 월드 드랍 메시 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="World Visual Fragment")
	TSoftObjectPtr<UStaticMesh> WorldMesh;

	/** 오버라이드 머티리얼(불필요할 경우 삭제) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="World Visual Fragment")
	TSoftObjectPtr<UMaterialInterface> OverrideMaterial;

	/** 메시의 Color 슬롯에 적용할 머티리얼 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="World Visual Fragment")
	TSoftObjectPtr<UMaterialInterface> ColorSlotMaterial;

	/** 월드 스케일 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="World Visual Fragment")
	FVector WorldScale;
};
