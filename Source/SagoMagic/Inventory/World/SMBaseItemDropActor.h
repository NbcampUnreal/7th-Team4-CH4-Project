#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/Core/SMItemDropTypes.h"
#include "SMBaseItemDropActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USMItemDefinition;
class USMWorldVisualFragment;


/**
 * 월드 드랍 아이템 액터 정의 파일
 *
 * 포함 내용:
 * - 드랍 Payload 보관
 * - Payload 기반 초기화
 * - 월드 비주얼 적용 함수
 * - 복제 상태 동기화 처리
 *
 * 역할:
 * - 인벤토리에서 제거된 아이템을 월드 드랍 액터 형태로 유지
 */

/** 월드 드랍 아이템 액터 */
UCLASS()
class SAGOMAGIC_API ASMBaseItemDropActor : public AActor
{
    GENERATED_BODY()

public:
    /** 기본 생성자 */
    ASMBaseItemDropActor();

    /** BeginPlay 오버라이드 */
    virtual void BeginPlay() override;

    /** 리플리케이션 프로퍼티 등록 오버라이드 */
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** 드랍 Payload Getter */
    const FSMItemDropPayload& GetItemDropPayload() const
    {
        return ItemDropPayload;
    }

    /** 루트 씬 컴포넌트 Getter */
    USceneComponent* GetRootSceneComponent() const
    {
        return RootSceneComponent;
    }

    /** 스태틱 메시 컴포넌트 Getter */
    UStaticMeshComponent* GetStaticMeshComponent() const
    {
        return StaticMeshComponent;
    }

    /** 초기화 여부 Getter */
    bool IsInitialized() const
    {
        return bInitialized;
    }

public:
    /** Payload 기반 초기화 요청 */
    UFUNCTION(BlueprintCallable, Category="Item Drop")
    void InitializeFromPayload(const FSMItemDropPayload& InItemDropPayload);

private:
    /** Payload 복제 수신 함수 */
    UFUNCTION()
    void OnRep_ItemDropPayload();

public:
    /** 아이템 정의 로드 */
    const USMItemDefinition* ResolveItemDefinition() const;

protected:
    /** 월드 비주얼 적용 */
    void ApplyWorldVisual();

    /** Payload 유효성 검사 */
    bool HasValidPayload() const;

private:
    /** 월드 비주얼 Fragment 조회 */
    const USMWorldVisualFragment* FindWorldVisualFragment(const USMItemDefinition* InItemDefinition) const;

public:
    /** 루트 씬 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Drop")
    TObjectPtr<USceneComponent> RootSceneComponent;

    /** 드랍 표시용 스태틱 메시 컴포넌트 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Drop")
    TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

protected:
    /** 드랍 아이템 Payload */
    UPROPERTY(ReplicatedUsing=OnRep_ItemDropPayload, VisibleAnywhere, BlueprintReadOnly, Category="Item Drop")
    FSMItemDropPayload ItemDropPayload;

private:
    /** 초기화 여부 */
    bool bInitialized;
};
