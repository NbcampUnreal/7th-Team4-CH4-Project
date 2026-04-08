#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "UI/SMEnemyHPBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "../GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "../Data/SMMonsterData.h"
#include "Core/DataManager/SMAsyncDataManager.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

ASMMonsterBase::ASMMonsterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    AIControllerClass = ASMMonsterAIController::StaticClass();
    
    //PreSpawn 시 AI가 즉시 시작되는 것을 방지
    //TickActivation에서 활성화할 때 SpawnDefaultController()로 수동 시작
    AutoPossessAI = EAutoPossessAI::Disabled;

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

    MonsterType = EMonsterType::None;

}

UAbilitySystemComponent* ASMMonsterBase::GetAbilitySystemComponent() const
{
    return MonsterAbilitySystemComponent;
}

void ASMMonsterBase::ResetMonster()
{
    SetActorEnableCollision(false);
    SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
}

void ASMMonsterBase::ApplyVisuals(USMMonsterDataAsset* DataAsset)
{
    if (!DataAsset) return;
    UE_LOG(LogTemp, Log, TEXT("[ApplyVisuals] DataAsset: %s"), *DataAsset->GetName());
    //TODO 은서 / 영택 : 추가적으로 넣어야 할 변수 넣어줘야함 
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (!DataAsset->SkeletalMesh.IsNull())
            MeshComp->SetSkeletalMesh(DataAsset->SkeletalMesh.LoadSynchronous());
        if (!DataAsset->AnimClass.IsNull())
            MeshComp->SetAnimInstanceClass(DataAsset->AnimClass.LoadSynchronous());
    }
}

void ASMMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASMMonsterBase, MonsterAssetId);
}

void ASMMonsterBase::OnRep_MonsterAssetId()
{
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    if (!AM) return;

    USMMonsterDataAsset* DataAsset = Cast<USMMonsterDataAsset>(AM->GetLoadAsset(MonsterAssetId));
    if (DataAsset)
        ApplyVisuals(DataAsset);
}

//void ASMMonsterBase::MulticastHandleDeath_Implementation()
//{
//    SetActorHiddenInGame(true);
//    SetActorEnableCollision(false);
//    SetActorTickEnabled(false);
//}

void ASMMonsterBase::BeginPlay()
{
	Super::BeginPlay();
    
    // TODO 현 : 클라이언트 전용 초기화 로직
    if (!HasAuthority() && MonsterAbilitySystemComponent)
    {
        // 클라이언트 측 ASC 액터 정보 초기화
        MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
    }
}
void ASMMonsterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    //UE_LOG(LogTemp, Warning, TEXT("[Monster] PossessedBy - DefaultAbilities 수: %d"), DefaultAbilities.Num());

    if (MonsterAbilitySystemComponent)
    {
        MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);


        if (!MonsterAttributeSet)
        {
            MonsterAttributeSet = const_cast<USMMonsterAttributeSet*>(
                MonsterAbilitySystemComponent->GetSet<USMMonsterAttributeSet>());
            UE_LOG(LogTemp, Warning, TEXT("[Monster] AttributeSet 재취득: %s"),
                MonsterAttributeSet ? TEXT("성공") : TEXT("실패"));
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

        if (GetCharacterMovement())
        {
            GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
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
        }
    }
}

void ASMMonsterBase::HandleDeath(AController* KillerController)
{
   
    // 이미 죽었거나 유효하지 않으면 무시
    if (!IsValid(this) || !HasAuthority()) return;

    // 막타 친 플레이어에게 골드 지급
    if (KillerController)
    {
        if (APawn* KillerPawn = KillerController->GetPawn())
        {
            if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(KillerPawn))
            {
                UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
                if (ASC)
                {
                    const USMPlayerAttributeSet* PlayerAttr =
                        ASC->GetSet<USMPlayerAttributeSet>();
                    if (PlayerAttr)
                    {
                        USMPlayerAttributeSet* MutableAttr =
                            const_cast<USMPlayerAttributeSet*>(PlayerAttr);

                        float GoldReward = MonsterAttributeSet->GetDropGold();
                        float NewGold = MutableAttr->GetGold() + GoldReward;
                        MutableAttr->SetGold(NewGold);
                       
                        //UE_LOG(LogTemp, Log, TEXT("[Gold] %s에게 %.0f Gold 지급 (총 %.0f)"),
                        //    *KillerController->GetName(), GoldReward, NewGold);
                    }
                }
            }
        }
    }

    if (USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this))
    {
        WM->OnMonsterDied(this);
    }
    
    // AI 정지
    if (ASMMonsterAIController* AICtl = Cast<ASMMonsterAIController>(GetController()))
    {
        AICtl->StopMovement();
    }
    // 이동 즉시 정지
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement(); // 이후 이동 입력 차단
    }

    // 충돌·이동 비활성화
    ResetMonster();
    //MulticastHandleDeath();

    // 3초 후 액터 제거 (애니메이션 붙일 자리)
    //SetLifeSpan(3.0f);

}
