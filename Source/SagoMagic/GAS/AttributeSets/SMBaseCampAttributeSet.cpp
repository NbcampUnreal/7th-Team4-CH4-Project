#include "SMBaseCampAttributeSet.h"
#include "Net/UnrealNetwork.h"
USMBaseCampAttributeSet::USMBaseCampAttributeSet()
{
	InitHealth(100.f);
}

void USMBaseCampAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	
}
void USMBaseCampAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 예시: Health라는 변수를 복제하고 싶다면 아래처럼 추가합니다.
	// DOREPLIFETIME(USMBaseCampAttributeSet, Health);
}
