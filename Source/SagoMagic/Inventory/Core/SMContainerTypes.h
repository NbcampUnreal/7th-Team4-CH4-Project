#pragma once

#include "CoreMinimal.h"
#include "Inventory/Core/SMInventoryCoreTypes.h"
#include "SMContainerTypes.generated.h"


/**
 * 인벤토리 컨테이너 및 퀵슬롯 상태 Struct 정의 파일
 *
 * 포함 내용:
 * - 배치형 인벤토리 컨테이너 상태
 * - 퀵슬롯 상태
 *
 * 역할:
 * - 메인 인벤토리, 스킬 내부 인벤토리, 퀵슬롯의 현재 상태를 표현하는 런타임 데이터 제공
 */

/** 배치형 인벤토리 컨테이너 상태 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMGridContainerState
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    FSMGridContainerState()
        : ContainerType(ESMContainerType::None)
    {
    }

    /** 컨테이너 ID Getter */
    const FGuid& GetContainerId() const
    {
        return ContainerId;
    }

    /** 컨테이너 종류 Getter */
    ESMContainerType GetContainerType() const
    {
        return ContainerType;
    }

    /** 유효 마스크 Getter */
    const FSMGridMaskData& GetValidMask() const
    {
        return ValidMask;
    }

    /** 컨테이너 ID Setter */
    void SetContainerId(const FGuid& InContainerId)
    {
        ContainerId = InContainerId;
    }

    /** 컨테이너 종류 Setter */
    void SetContainerType(const ESMContainerType InContainerType)
    {
        ContainerType = InContainerType;
    }

    /** 유효 마스크 Setter */
    void SetValidMask(const FSMGridMaskData& InValidMask)
    {
        ValidMask = InValidMask;
    }

public:
    /** 컨테이너 유효성 검사 */
    bool IsValidContainer() const
    {
        return ContainerId.IsValid() && ValidMask.IsValidMaskData();
    }

public:
    /** 컨테이너 고유 ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Container")
    FGuid ContainerId;

    /** 컨테이너 종류 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Container")
    ESMContainerType ContainerType;

    /** 활성 칸 마스크 데이터 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Container")
    FSMGridMaskData ValidMask;

    /** 포함 아이템 인스턴스 ID 목록 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Container")
    TArray<FGuid> ContainedItemIds;
};

/** 퀵슬롯 상태 데이터(추후 로직 재정의 필요 가능성 있음) */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMQuickSlotSetState
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    FSMQuickSlotSetState()
        : ActiveSlotIndex(0)
    {
    }

    /** 활성 슬롯 인덱스 Getter */
    int32 GetActiveSlotIndex() const
    {
        return ActiveSlotIndex;
    }

    /** 활성 슬롯 인덱스 Setter */
    void SetActiveSlotIndex(const int32 InActiveSlotIndex)
    {
        ActiveSlotIndex = InActiveSlotIndex;
    }

public:
    /** 1번 슬롯 스킬 인스턴스 ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|QuickSlot")
    FGuid Slot1SkillId;

    /** 2번 슬롯 스킬 인스턴스 ID */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|QuickSlot")
    FGuid Slot2SkillId;

    /** 현재 활성 슬롯 인덱스 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|QuickSlot")
    int32 ActiveSlotIndex;
};
