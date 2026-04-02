#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterAIController.h"
#include "AbilitySystemComponent.h"

ASMMonsterBase::ASMMonsterBase()
{
    AIControllerClass = ASMMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // ASC 생성
    MonsterAbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    // 서버-클라이언트 복제 설정
    MonsterAbilitySystemComponent->SetIsReplicated(true);
    // 몬스터는 보통 혼합(Mixed) 모드나 미니멀(Minimal) 복제 모드를 사용합니다.
    MonsterAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

    // AttributeSet 생성
    MonsterAttributeSet = CreateDefaultSubobject<USMMonsterAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ASMMonsterBase::GetAbilitySystemComponent() const
{
    return nullptr;
}

void ASMMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
void ASMMonsterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 서버에서 ASC 초기화
    if (MonsterAbilitySystemComponent)
    {
        MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
    }

    // 여기서 초기 스킬(Ability)을 부여하거나 스탯 기본값을 설정
}
//void ASMMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//    // Health 변수를 복제 대상으로 등록
//    DOREPLIFETIME(ASMMonsterBase, Health);
//}
//
//void ASMMonsterBase::OnRepHealth()
//{
//
//}

