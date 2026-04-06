#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "UI/SMEnemyHPBarComponent.h"


ASMMonsterBase::ASMMonsterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    AIControllerClass = ASMMonsterAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 서버 통신 활성화
    bReplicates = true;
    // ASC 생성
    MonsterAbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    // 서버-클라이언트 복제 설정
    MonsterAbilitySystemComponent->SetIsReplicated(true);
    // 몬스터는 보통 혼합(Mixed) 모드나 미니멀(Minimal) 복제 모드를 사용합니다.
    // 몬스터는 보통 서버에서만 판정하므로 Minimal 모드로 설정하여 네트워크 대역폭 절약
    MonsterAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    // AttributeSet 생성
    MonsterAttributeSet = CreateDefaultSubobject<USMMonsterAttributeSet>(TEXT("AttributeSet"));

}

UAbilitySystemComponent* ASMMonsterBase::GetAbilitySystemComponent() const
{
    return MonsterAbilitySystemComponent;
}

void ASMMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
void ASMMonsterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //UE_LOG(LogTemp, Warning, TEXT("[Monster] PossessedBy - DefaultAbilities 수: %d"), DefaultAbilities.Num());

    if (MonsterAbilitySystemComponent)
    {
        MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
        
        // TODO 현 : HP바 컴포넌트 찾아 ASC 알림
        USMEnemyHPBarComponent* HPBarComp = FindComponentByClass<USMEnemyHPBarComponent>();
        if (HPBarComp)
        {
            HPBarComp->InitializeHPBar(MonsterAbilitySystemComponent);
        }

        if (HasAuthority() && MonsterAttributeSet)
        {
            MonsterAttributeSet->OnMonsterDied.AddUObject(this, &ASMMonsterBase::HandleDeath);
        }

        GiveDefaultAbilities();

        if (ASMMonsterAIController* MonsterAI = Cast<ASMMonsterAIController>(NewController))
        {
            MonsterAI->StartAttackTimer();
        }
    }
}

void ASMMonsterBase::GiveDefaultAbilities()
{
    if (!HasAuthority() || !MonsterAbilitySystemComponent)
    {
        return;
    }

    for (TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            FGameplayAbilitySpec Spec(AbilityClass, 1);
            MonsterAbilitySystemComponent->GiveAbility(Spec);

           /* UE_LOG(LogTemp, Warning, TEXT("[Monster] %s / ASC:%p / 어빌리티 부여: %s"),
                *GetName(),
                MonsterAbilitySystemComponent.Get(),
                *AbilityClass->GetName());*/
        }
    }

   /* UE_LOG(LogTemp, Warning, TEXT("[Monster] %s / ASC:%p / 실제 부여된 수: %d"),
        *GetName(),
        MonsterAbilitySystemComponent.Get(),
        MonsterAbilitySystemComponent->GetActivatableAbilities().Num());*/
}


void ASMMonsterBase::HandleDeath()
{
    if (MonsterAttributeSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterHP] %s / HP: %.1f"),
            *GetName(),
            MonsterAttributeSet->GetHealth());
    }
    // 이미 죽었거나 유효하지 않으면 무시
    if (!IsValid(this) || !HasAuthority())
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Monster] %s 사망. 3초 후 제거됩니다."), *GetName());

    // AI 정지
    if (AController* MyController = GetController())
    {
        MyController->UnPossess();
    }

    // 충돌·이동 비활성화
    SetActorEnableCollision(false);

    // 3초 후 액터 제거 (애니메이션 붙일 자리)
    SetLifeSpan(3.0f);
}