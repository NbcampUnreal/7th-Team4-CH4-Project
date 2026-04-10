#include "GameplayTags/UI/SMUITag.h"

namespace SMUITag
{
    /**
     *사용 양식
     *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
     *예시
     *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
     */
	
	// 별명과 실제 태그 문자열 연결
	UE_DEFINE_GAMEPLAY_TAG(Event_BaseCamp, "UI.Event.BaseCamp");
}
