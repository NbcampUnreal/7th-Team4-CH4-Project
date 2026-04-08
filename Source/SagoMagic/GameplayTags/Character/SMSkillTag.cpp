#include "SMSkillTag.h"

namespace SMSkillTag
{
	/**
	 *사용 양식
	 *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
	 *예시
	 *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
	 */

	//스킬 태그
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill, TEXT("Ability.Skill"))
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_Projectile, TEXT("Ability.Skill.Projectile"))
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_SpawnField, TEXT("Ability.Skill.SpawnField"))
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_LineTrace, TEXT("Ability.Skill.LineTrace"))
	UE_DEFINE_GAMEPLAY_TAG(Ability_Skill_ApplyInstantDamage, TEXT("Ability.Skill.ApplyInstantDamage"))

	//쿨다운 태그들
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_Projectile, TEXT("Cooldown.Skill.Projectile"))
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_SpawnField, TEXT("Cooldown.Skill.SpawnField"))
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_LineTrace, TEXT("Cooldown.Skill.LineTrace"))
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Skill_ApplyInstantDamage, TEXT("Cooldown.Skill.ApplyInstantDamage"))

	//GE Spec 태그들
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage_Amount, TEXT("Data.Damage.Amount"))
	UE_DEFINE_GAMEPLAY_TAG(Data_Cooldown, TEXT("Data.Cooldown"))

	//Cue 태그
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_Projectile_Hit, TEXT("GameplayCue.Skill.Projectile_Hit"))
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_SpawnField_Tick, TEXT("GameplayCue.Skill.SpawnField_Tick"))
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_LineTrace_Hit, TEXT("GameplayCue.Skill.LineTrace.Hit"))
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_LineTrace_Beam, TEXT("GameplayCue.Skill.LineTrace.Beam"))
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Skill_ApplyInstantDamage_Hit, TEXT("GameplayCue.Skill.ApplyInstantDamage_Hit"))
}
