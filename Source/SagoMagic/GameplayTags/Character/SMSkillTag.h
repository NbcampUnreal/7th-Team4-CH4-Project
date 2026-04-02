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

    //아군 태그 - 아군의 공격이 아군이나 HQ를 임하지 못하게
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Player)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_HQ)

    //Cue 태그
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_Projectile_Hit)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_SpawnField_Tick)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_LineTrace_Hit)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Skill_ApplyInstantDamage_Hit)

}
