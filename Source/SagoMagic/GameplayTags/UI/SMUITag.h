#pragma once

#include "NativeGameplayTags.h"

namespace SMUITag
{
    /**
    *사용 양식
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(간편하게 사용할 태그이름);
    *예시
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Reload_Ammo);
    */
    
    /** 베이스캠프 UI 이벤트 태그 선언 */
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_BaseCamp);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wave);
}
