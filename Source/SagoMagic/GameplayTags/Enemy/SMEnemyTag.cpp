#include "GameplayTags/Enemy/SMEnemyTag.h"

namespace SMEnemyTag
{
    /**
     *사용 양식
     *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
     *예시
     *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
     */
    UE_DEFINE_GAMEPLAY_TAG(Enemy_HitEvent, "Enemy.Hit");
}
