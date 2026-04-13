#pragma once

#include "NativeGameplayTags.h"

namespace SMCharacterTag
{
	/**
	*사용 양식
	*UE_DECLARE_GAMEPLAY_TAG_EXTERN(간편하게 사용할 태그이름);
	*예시
	*UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Reload_Ammo);
	*/

	/** 캐릭터의 상태 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Build_Place)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Build_Edit)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking)
	
	/** 디폴트 어빌리티 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Default_Interact)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Default_Pickup)

	/** 건축 어빌리티 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Build_Place)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Build_GoldCost)
	
}
