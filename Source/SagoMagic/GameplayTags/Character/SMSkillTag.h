#pragma once

#include "NativeGameplayTags.h"

namespace SMSkillTag
{
    /**
    *사용 양식
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(간편하게 사용할 태그이름);
    *예시
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Reload_Ammo);
    */

    //태그명 추천받습니다.

    //스킬 태그들
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill)
    //투사체
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_Projectile)
    //장판
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_SpawnField)
    //빔
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_LineTrace)
    //레이
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Skill_ApplyInstantDamage)

    //쿨다운 태그들
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_Projectile)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_SpawnField)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_LineTrace)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Skill_ApplyInstantDamage)

    //Set By Caller 데이터 태그
    //UGE_SkillDamage 에서 피해량
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage_Amount)
    //UGE_SkillCooldown - Duration
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Cooldown)

    //코스메틱 태그
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_Projectile_Hit)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_SpawnField_Tick)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_LineTrace_Hit)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_LineTrace_Beam)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_ApplyInstantDamage_Hit)

}
