#pragma once

#include "NativeGameplayTags.h"

namespace SMGameFlowTag
{
	//아군 태그 - 아군의 공격이 아군이나 HQ, 아군의 구조물을 공격하지 못하게
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Player)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_HQ)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Building)

}
