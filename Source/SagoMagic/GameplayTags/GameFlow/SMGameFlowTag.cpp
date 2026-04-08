#include "SMGameFlowTag.h"

namespace SMGameFlowTag
{
	/**
	 *사용 양식
	 *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
	 *예시
	 *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
	 */

	//아군 태그
	UE_DEFINE_GAMEPLAY_TAG(Team, TEXT("Team"))
	UE_DEFINE_GAMEPLAY_TAG(Team_Player, TEXT("Team.Player"))
	UE_DEFINE_GAMEPLAY_TAG(Team_HQ, TEXT("Team.HQ"))
	UE_DEFINE_GAMEPLAY_TAG(Team_Building, TEXT("Team.Building"))

}
