#pragma once

#include "CoreMinimal.h"
#include "UGE_SkillDamage.h"
#include "UGE_InstantDamage.generated.h"

/**
 *  SkillDamage   플레이어 — 스킬이 피해를 발생시킴
 *  InstantDamage 몬스터 — IncomingDamage 에 피해량 기록 + GC 트리거
 *
 * 흐름
 *  USMAbilitySystemComponent::ApplySkillDamage()
 *    -> UGE_InstantDamage Spec 생성
 *    -> SetByCaller(Data.Damage.Amount) = 피해량 넣기
 *    -> TargetASC에 적용
 *    -> USMAttributeSet::PostGameplayEffectExecute()
 *         IncomingDamage -> Health 차감 후 0 초기화
 *    -> GameplayCues 자동 트리거 -> 피격 이펙트/사운드 코스메틱 효과 재생
 *
 * Cues
 *  GameplayCues 배열에 원하는 GC 태그 추가
 *  (예: GameplayCue.Skill.Projectile.Hit)
 */
UCLASS()
class SAGOMAGIC_API UGE_InstantDamage : public UGE_SkillDamage
{
    GENERATED_BODY()

public:
    UGE_InstantDamage();
};
