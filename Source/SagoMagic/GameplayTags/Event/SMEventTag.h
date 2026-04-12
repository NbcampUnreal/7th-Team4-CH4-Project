#pragma once

#include "NativeGameplayTags.h"

namespace SMEventTag
{
    /**
    *사용 양식
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(간편하게 사용할 태그이름);
    *예시
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Reload_Ammo);
    */
	//AnimNotify → GAS 이벤트 전달
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Monster_HitCheck);
	
	// 플레이어 스킬 AnimNotify 이벤트 전달용
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Skill_Projectile);
}
