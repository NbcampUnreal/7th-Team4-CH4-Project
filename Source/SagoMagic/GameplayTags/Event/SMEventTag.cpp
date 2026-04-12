#include "GameplayTags/Event/SMEventTag.h"

namespace SMEventTag
{
    /**
     *사용 양식
     *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
     *예시
     *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
     */

    //AnimNotify → GAS 이벤트 전달
    UE_DEFINE_GAMEPLAY_TAG(Event_Monster_HitCheck, "Event.Monster.HitCheck");
	
	// 플레이어 스킬 AnimNotify 이벤트 전달용
	UE_DEFINE_GAMEPLAY_TAG(Event_Skill_Projectile, "Event.Skill.Projectile");
}
