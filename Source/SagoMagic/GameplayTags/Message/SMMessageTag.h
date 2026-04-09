#pragma once

#include "NativeGameplayTags.h"

namespace SMMessageTag
{
    /**
    *사용 양식
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(간편하게 사용할 태그이름);
    *예시
    *UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Reload_Ammo);
    */

    /** 인벤토리 갱신 메시지 채널 태그 */
    SAGOMAGIC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_Updated);

    /** 메인 인벤토리 갱신 메시지 채널 태그 */
    SAGOMAGIC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_MainContainerUpdated);

    /** 스킬 내부 인벤토리 갱신 메시지 채널 태그 */
    SAGOMAGIC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_SkillContainerUpdated);

    /** 스킬 요약 갱신 메시지 채널 태그 */
    SAGOMAGIC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_SkillSummaryUpdated);

    /** 퀵슬롯 갱신 메시지 채널 태그 */
    SAGOMAGIC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Inventory_QuickSlotUpdated);
}
