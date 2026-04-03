#include "GameplayTags/Character/SMCharacterTag.h"

namespace SMCharacterTag
{
	/**
	 *사용 양식
	 *UE_DEFINE_GAMEPLAY_TAG(간편하게 사용할 태그이름, 해당 이름과 연동될 실제 GameplayTag);
	 *예시
	 *UE_DEFINE_GAMEPLAY_TAG(Data_Reload_Ammo, "Data.Reload.Ammo");
	 */

	/** 캐릭터의 상태 */
	UE_DEFINE_GAMEPLAY_TAG(State_Combat, TEXT("State.Combat"))
	UE_DEFINE_GAMEPLAY_TAG(State_Construct, TEXT("State.Construct"))
	
	/** 디폴트 어빌리티 */
	UE_DEFINE_GAMEPLAY_TAG(Ability_Default_Interact, TEXT("Ability.Default.Interact"))
	UE_DEFINE_GAMEPLAY_TAG(Ability_Default_Pickup, TEXT("Ability.Default.Pickup"))
}
