#include "Enemy/SMMonsterBase.h"
#include "Enemy/SMMonsterAIController.h"
#include "AbilitySystemComponent.h"
#include "UI/SMEnemyHPBarComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "../GAS/AttributeSets/SMPlayerAttributeSet.h"
#include "../Data/SMMonsterData.h"
#include "Core/DataManager/SMAsyncDataManager.h"
#include "Core/DataManager/SMSyncDataManager.h"
#include "Core/Wave/SMWaveManagerSubsystem.h"
#include "Core/SMGameMode.h"
#include "Engine/AssetManager.h"
#include "Inventory/World/SMBaseItemDropActor.h"
#include "Inventory/Core/SMItemDropTypes.h"
#include "Data/SMItemDropTableData.h"
#include "Inventory/Items/Definitions/SMGemItemDefinition.h"
#include "Inventory/Items/Definitions/SMSkillItemDefinition.h"
#include "Net/UnrealNetwork.h"
#include "Building/SMBaseCampActor.h"
#include "GameplayTags/Enemy/SMEnemyTag.h"
#include "GameplayTags/GameFlow/SMGameFlowTag.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayEffect.h"
#include "Animation/AnimMontage.h"
#include "Core/DataManager/SMSoundManager.h"


ASMMonsterBase::ASMMonsterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    //AIControllerClass = ASMMonsterAIController::StaticClass();
    
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
    //TODO 은서 / 영택 : 추가적으로 넣어야 할 변수 넣어줘야함 material 추가
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (!DataAsset->SkeletalMesh.IsNull())
            MeshComp->SetSkeletalMesh(DataAsset->SkeletalMesh.LoadSynchronous());
        if (!DataAsset->AnimClass.IsNull())
            MeshComp->SetAnimInstanceClass(DataAsset->AnimClass.LoadSynchronous());
    }

    // AttackMontage 캐싱 (클라이언트에서 애니메이션 재생에 필요)
    if (!DataAsset->AttackMontage.IsNull())
    {
        CachedAttackMontage = DataAsset->AttackMontage.LoadSynchronous();
    }
}

void ASMMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASMMonsterBase, MonsterAssetId);
    DOREPLIFETIME(ASMMonsterBase, bIsDead);
}

void ASMMonsterBase::ApplyLogicData(USMMonsterDataAsset* DataAsset)
{
    if (!DataAsset || !HasAuthority()) return;

    // 1) DefaultAbilities 세팅 (GiveDefaultAbilities 호출 전에)
    DefaultAbilities = DataAsset->DefaultAbilities;

    // 2) DamageEffect 캐싱
    if (!DataAsset->DamageEffect.IsNull())
    {
        CachedDamageEffect = DataAsset->DamageEffect.LoadSynchronous();
    }

    // 3) BehaviorTree → AIController에 전달
    if (!DataAsset->BehaviorTree.IsNull())
    {
        UBehaviorTree* BT = DataAsset->BehaviorTree.LoadSynchronous();
        if (BT)
        {
            if (ASMMonsterAIController* AICtl = Cast<ASMMonsterAIController>(GetController()))
            {
                // BT 내장 BlackboardAsset 사용
                AICtl->InitFromDataAsset(BT, BT->BlackboardAsset);
            }
        }
    }
}

void ASMMonsterBase::OnRep_MonsterAssetId()
{
    USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
    if (!AM) return;

    USMMonsterDataAsset* DataAsset = Cast<USMMonsterDataAsset>(AM->GetLoadAsset(MonsterAssetId));
    if (DataAsset)
        ApplyVisuals(DataAsset);
}

void ASMMonsterBase::SelfKill()
{
    if (!HasAuthority() || !IsValid(this)) return;

    // ── 1) GameMode에서 BaseCamp 즉시 획득 (O(1)) ──
    ASMGameMode* GM = GetWorld()->GetAuthGameMode<ASMGameMode>();
    ASMBaseCampActor* TargetCamp = GM ? GM->GetBaseCamp() : nullptr;

    // 파괴됐거나 무효하면 데미지 대상에서 제외 (몬스터 제거는 계속 진행)
    if (IsValid(TargetCamp) && TargetCamp->GetCurrentHealth() > 0.f)
    {
        // ── 2) BaseCamp AttributeSet에 직접 HP 차감 ──
        if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(TargetCamp))
        {
            if (UAbilitySystemComponent* TargetASC = ASCInterface->GetAbilitySystemComponent())
            {
                if (USMBaseCampAttributeSet* CampAttr = const_cast<USMBaseCampAttributeSet*>(
                    TargetASC->GetSet<USMBaseCampAttributeSet>()))
                {
                    const float OldHealth = CampAttr->GetHealth();
                    const float NewHealth = FMath::Clamp(OldHealth - SelfKillDamage, 0.f, CampAttr->GetMaxHealth());
                    CampAttr->SetHealth(NewHealth);

                    //UE_LOG(LogTemp, Log, TEXT("[SelfKill] %s → BaseCamp HP: %.1f → %.1f"),
                    //    *GetName(), OldHealth, NewHealth);

                    // HP 0 도달 시 GameMode에 패배 처리 요청
                    // (PostGameplayEffectExecute를 우회했으므로 여기서 명시적으로 호출)
                    if (NewHealth <= 0.f && OldHealth > 0.f && GM)
                    {
                        GM->OnBaseCampDestroyed();
                    }
                }
            }
        }
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("[SelfKill] %s - BaseCamp 없음/파괴됨"), *GetName());
    }

    // ── 3) AI / 이동 정리 ──
    if (ASMMonsterAIController* AICtl = Cast<ASMMonsterAIController>(GetController()))
    {
        AICtl->StopMovement();
        AICtl->StopAttackTimer();
    }
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->StopMovementImmediately();
        MoveComp->DisableMovement();
    }

    ResetMonster();

    // ── 4) WaveManager에 알림 → ScheduleDestroy 경로로 안전하게 제거 ──
    if (USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this))
    {
        WM->OnMonsterDied(this);
    }
}

void ASMMonsterBase::BeginPlay()
{
	Super::BeginPlay();
    if (!MonsterAbilitySystemComponent)
    {
        MonsterAbilitySystemComponent = FindComponentByClass<UAbilitySystemComponent>();
    }

    // TODO 현 : 클라이언트 전용 초기화 로직
    if (!HasAuthority() && MonsterAbilitySystemComponent)
    {
        // 클라이언트 측 ASC 액터 정보 초기화
        MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
        MonsterAbilitySystemComponent->AddLooseGameplayTag(SMGameFlowTag::Enemy);
    }
}
void ASMMonsterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // TObjectPtr 레졸브 실패 대비 — 컴포넌트 직접 검색으로 복구
    if (!MonsterAbilitySystemComponent)
    {
        MonsterAbilitySystemComponent = FindComponentByClass<UAbilitySystemComponent>();
    }
    if (!MonsterAbilitySystemComponent) return;

    MonsterAbilitySystemComponent->InitAbilityActorInfo(this, this);
    MonsterAbilitySystemComponent->AddLooseGameplayTag(SMGameFlowTag::Enemy);

    if (!MonsterAttributeSet)
    {
        MonsterAttributeSet = const_cast<USMMonsterAttributeSet*>(
            MonsterAbilitySystemComponent->GetSet<USMMonsterAttributeSet>());
        //UE_LOG(LogTemp, Warning, TEXT("[Monster] AttributeSet 재취득: %s"),
        //    MonsterAttributeSet ? TEXT("성공") : TEXT("실패"));
    }

    if (HasAuthority() && MonsterAttributeSet)
    {
        MonsterAttributeSet->OnMonsterDied.RemoveAll(this);
        MonsterAttributeSet->OnMonsterDied.AddUObject(this, &ASMMonsterBase::HandleDeath);
    }

    // ── DataAsset에서 로직 데이터 적용 ──
    if (HasAuthority())
    {
        USMAsyncDataManager* AM = USMAsyncDataManager::Get(this);
        //UE_LOG(LogTemp, Warning, TEXT("[PossessedBy] AM: %s, AssetId: %s"),
        //    AM ? TEXT("Valid") : TEXT("NULL"),
        //    *MonsterAssetId.ToString());
        if (AM && MonsterAssetId.IsValid())
        {
            USMMonsterDataAsset* DataAsset = Cast<USMMonsterDataAsset>(
                AM->GetLoadAsset(MonsterAssetId));
            //UE_LOG(LogTemp, Warning, TEXT("[PossessedBy] DataAsset: %s"),
            //    DataAsset ? *DataAsset->GetName() : TEXT("NULL"));
            if (DataAsset)
            {
                ApplyLogicData(DataAsset);
                ApplyVisuals(DataAsset);
            }
        }
    }

    GiveDefaultAbilities();
    //UE_LOG(LogTemp, Warning, TEXT("[PossessedBy] Abilities 부여 완료: %d개"), DefaultAbilities.Num());

    if (ASMMonsterAIController* MonsterAI = Cast<ASMMonsterAIController>(NewController))
    {
        MonsterAI->StartAttackTimer();
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MonsterAttributeSet->GetMoveSpeed();
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

    bIsDead = true;
    MonsterAbilitySystemComponent->AddLooseGameplayTag(SMEnemyTag::Enemy_State_Death);
    // AnimInstance에 사망 알리기
    //if (USMMonsterAnimInstance* AnimInst =
    //    Cast<USMMonsterAnimInstance>(GetMesh()->GetAnimInstance()))
    //{
    //    AnimInst->bIsDead = true;
    //}

    // 사망 Montage 재생
    //if (DeathMontage)
    //{
    //    PlayAnimMontage(DeathMontage);
    //}

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
    // ── 아이템 드롭 (공용 드롭 테이블에서 가중치 기반 1개) ──
    SpawnDropItem();

    if (USMWaveManagerSubsystem* WM = USMWaveManagerSubsystem::Get(this))
    {
        WM->OnMonsterDied(this);
    }
    
    // AI 정지
    if (ASMMonsterAIController* AICtl = Cast<ASMMonsterAIController>(GetController()))
    {
        AICtl->StopMovement();
        AICtl->StopAttackTimer(); // 추가
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

void ASMMonsterBase::SpawnDropItem()
{
    if (!HasAuthority()) return;

    USMSyncDataManager* DM = USMSyncDataManager::Get(this);
    if (!DM) return;

    const TMap<TSoftObjectPtr<USMItemDefinition>, FSMItemDropTableData>& DropTable =
        DM->GetItemDropTableCache();

    if (DropTable.Num() == 0) return;

    // 1) 유효한 항목의 가중치 합산
    int32 TotalWeight = 0;
    for (const auto& Pair : DropTable)
    {
        if (Pair.Value.IsValidData())
        {
            TotalWeight += Pair.Value.GetDropWeight();
        }
    }
    if (TotalWeight <= 0) return;

    // 2) 가중치 기반 랜덤 1개 선택
    int32 Roll = FMath::RandRange(0, TotalWeight - 1);
    int32 Accumulated = 0;
    TSoftObjectPtr<USMItemDefinition> SelectedItem;

    for (const auto& Pair : DropTable)
    {
        if (!Pair.Value.IsValidData()) continue;

        Accumulated += Pair.Value.GetDropWeight();
        if (Roll < Accumulated)
        {
            SelectedItem = Pair.Key;
            break;
        }
    }
    if (SelectedItem.IsNull()) return;
    
    USMItemDefinition* SelectedItemDefinition = SelectedItem.LoadSynchronous();
    if (SelectedItemDefinition == nullptr) return;

    ESMItemType ResolvedItemType = ESMItemType::None;
    if (SelectedItemDefinition->IsA<USMSkillItemDefinition>())
    {
        ResolvedItemType = ESMItemType::Skill;
    }
    else if (SelectedItemDefinition->IsA<USMGemItemDefinition>())
    {
        ResolvedItemType = ESMItemType::Gem;
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("[DropItem] Unsupported item definition type: %s"), *SelectedItemDefinition->GetClass()->GetName());
        return;
    }

    // 3) FSMItemDropPayload 생성
    FSMItemDropPayload Payload;
    Payload.SetInstanceId(FGuid::NewGuid());
    Payload.SetDefinition(SelectedItem);
    Payload.ItemType = ResolvedItemType;
    // 몬스터 드롭이므로 Rotation, bLocked, NestedItemSnapshots는 기본값 유지

    // 4) 월드에 ASMBaseItemDropActor 스폰
    FVector SpawnLocation = GetActorLocation();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    UClass* ActorClassToSpawn = DropActorClass.Get();
    if (ActorClassToSpawn == nullptr)
    {
        ActorClassToSpawn = ASMBaseItemDropActor::StaticClass();
    }

    ASMBaseItemDropActor* DroppedActor = GetWorld()->SpawnActor<ASMBaseItemDropActor>(
        ActorClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);

    if (DroppedActor)
    {
        DroppedActor->InitializeFromPayload(Payload);
        //UE_LOG(LogTemp, Log, TEXT("[DropItem] %s 사망 → %s 드롭"),
        //    *GetName(), *SelectedItem.ToString());
    }
}

void ASMMonsterBase::OnRep_IsDead()
{
    if (bIsDead) HandleClientDeath();
}

void ASMMonsterBase::HandleClientDeath()
{
    MonsterAbilitySystemComponent->AddLooseGameplayTag(SMEnemyTag::Enemy_State_Death);

    // 사망 사운드 재생
    if (!DeathSoundID.IsNone())
    {
        if (USMSoundManager* SM = USMSoundManager::Get(this))
        {
            SM->PlaySoundAtLocation(DeathSoundID, GetActorLocation());
        }
    }
}

void ASMMonsterBase::PlayHitSound()
{
    if (!HitSoundID.IsNone())
    {
        if (USMSoundManager* SM = USMSoundManager::Get(this))
        {
            SM->PlaySoundAtLocation(HitSoundID, GetActorLocation());
        }
    }
}

void ASMMonsterBase::Multicast_PlayAttackSound_Implementation()
{
    if (!AttackSoundID.IsNone())
    {
        if (USMSoundManager* SM = USMSoundManager::Get(this))
        {
            SM->PlaySoundAtLocation(AttackSoundID, GetActorLocation());
        }
    }
}
