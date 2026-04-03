#pragma once

#include "CoreMinimal.h"
#include "UI/Inventory/SMInventoryGridWidget.h"
#include "SMSkillInventoryWidget.generated.h"

class USMInventoryComponent;


/**
 * 스킬 내부 인벤토리 위젯 정의 파일
 *
 * 포함 내용:
 * - 표시 대상 스킬 인스턴스 ID
 * - 스킬 내부 컨테이너 표시 함수
 * - 선택된 스킬 상태 갱신 함수
 *
 * 역할:
 * - 선택된 스킬의 내부 인벤토리를 표시
 */

/** 스킬 내부 인벤토리 위젯 */
UCLASS()
class SAGOMAGIC_API USMSkillInventoryWidget : public USMInventoryGridWidget
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	USMSkillInventoryWidget(const FObjectInitializer& ObjectInitializer);

	/** 스킬 인스턴스 ID Getter */
	const FGuid& GetSkillInstanceId() const
	{
		return SkillInstanceId;
	}

	/** 스킬 인스턴스 ID Setter */
	void SetSkillInstanceId(const FGuid& InSkillInstanceId)
	{
		SkillInstanceId = InSkillInstanceId;
	}

public:
	/** 스킬 인벤토리 위젯 초기화 요청 */
	UFUNCTION(BlueprintCallable, Category="Skill Inventory Widget")
	void InitializeSkillInventoryWidget(
		const FGuid& InSkillInstanceId,
		const FGuid& InContainerId,
		USMInventoryComponent* InInventoryComponent);

	/** 표시 대상 스킬 변경 요청 */
	UFUNCTION(BlueprintCallable, Category="Skill Inventory Widget")
	void ChangeTargetSkill(const FGuid& InSkillInstanceId, const FGuid& InContainerId);

	/** 선택된 스킬 해제 요청 */
	UFUNCTION(BlueprintCallable, Category="Skill Inventory Widget")
	void ClearTargetSkill();

protected:
	/** 스킬 인벤토리 갱신 블루프린트 이벤트 */
	UFUNCTION(BlueprintImplementableEvent, Category="Skill Inventory Widget")
	void BP_OnSkillInventoryUpdated();

public:
protected:
	/** 표시 대상 스킬 인스턴스 ID */
	UPROPERTY(BlueprintReadOnly, Category="Skill Inventory Widget")
	FGuid SkillInstanceId;

private:
};
