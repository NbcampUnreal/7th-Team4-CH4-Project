#include "GameplayTags/Message/SMMessageTag.h"

namespace SMMessageTag
{
    /**
     *사용 양식
     *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
     *예시
     *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
     */

    /** 인벤토리 갱신 메시지 채널 태그 정의 */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_Updated, "SM.Message.Inventory.Updated");

    /** 메인 인벤토리 갱신 메시지 채널 태그 정의 */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_MainContainerUpdated, "SM.Message.Inventory.MainContainerUpdated");

    /** 스킬 내부 인벤토리 갱신 메시지 채널 태그 정의 */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_SkillContainerUpdated, "SM.Message.Inventory.SkillContainerUpdated");

    /** 스킬 요약 갱신 메시지 채널 태그 정의 */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_SkillSummaryUpdated, "SM.Message.Inventory.SkillSummaryUpdated");

    /** 퀵슬롯 갱신 메시지 채널 태그 정의 */
    UE_DEFINE_GAMEPLAY_TAG(Inventory_QuickSlotUpdated, "SM.Message.Inventory.QuickSlotUpdated");
}
