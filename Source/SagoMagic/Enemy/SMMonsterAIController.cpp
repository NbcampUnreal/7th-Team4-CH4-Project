#include "Enemy/SMMonsterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AbilitySystemComponent.h"
#include "Enemy/SMMonsterBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Building/SMBaseCampActor.h"
#include "Building/SMBaseBuilding.h"
#include "BehaviorTree/BehaviorTree.h"

ASMMonsterAIController::ASMMonsterAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASMMonsterAIController::OnTargetDetected);
}

void ASMMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BBAsset && BTAsset)
    {
        UBlackboardComponent* BlackboardCompPointer = BlackboardComp.Get();
        if (UseBlackboard(BBAsset, BlackboardCompPointer))
        {
            BlackboardComp = BlackboardCompPointer;

            // Blackboard의 TargetActor는 항상 BaseCamp
            AActor* BaseCamp = FindBaseCamp();
            if (BaseCamp)
            {
                Blackboard->SetValueAsObject(FName("TargetActor"), BaseCamp);
            }
            CurrentTargetType = EMonsterAttackTargetType::BaseCamp;

            RunBehaviorTree(BTAsset);
        }
    }
    else
    {
        //UE_LOG(LogTemp, Warning, TEXT("[AI] OnPossess - BBAsset: %s / BTAsset: %s"),
        //    BBAsset ? TEXT("Valid") : TEXT("NULL"),
        //    BTAsset ? TEXT("Valid") : TEXT("NULL"));
    }
}

void ASMMonsterAIController::OnUnPossess()
{
    Super::OnUnPossess();
    GetWorldTimerManager().ClearTimer(AttackCheckTimerHandle);
}
void ASMMonsterAIController::StopAttackTimer()
{
    GetWorldTimerManager().ClearTimer(AttackCheckTimerHandle);
}
void ASMMonsterAIController::StartAttackTimer()
{
    // 이미 타이머가 돌고 있으면 중복 실행 방지
    if (GetWorldTimerManager().IsTimerActive(AttackCheckTimerHandle)) return;

    GetWorldTimerManager().SetTimer(
        AttackCheckTimerHandle,
        this,
        &ASMMonsterAIController::UpdateTargetAndTryAttack,
        AttackCooldown,
        true
    );
    //UE_LOG(LogTemp, Warning, TEXT("[AI] 공격 타이머 시작. 쿨다운: %.1f초"), AttackCooldown);
}

void ASMMonsterAIController::InitFromDataAsset(UBehaviorTree* InBT, UBlackboardData* InBB)
{
    if (!InBT) return;

    // BB가 따로 안 넘어왔으면 BT에 내장된 BB 사용
    UBlackboardData* ResolvedBB = InBB ? InBB : InBT->GetBlackboardAsset();
    if (!ResolvedBB) return;

    BTAsset = InBT;
    BBAsset = ResolvedBB;

    UBlackboardComponent* BBCompPtr = BlackboardComp.Get();
    if (UseBlackboard(BBAsset, BBCompPtr))
    {
        BlackboardComp = BBCompPtr;

        AActor* BaseCamp = FindBaseCamp();
        if (BaseCamp)
        {
            Blackboard->SetValueAsObject(FName("TargetActor"), BaseCamp);
        }
        CurrentTargetType = EMonsterAttackTargetType::BaseCamp;

        RunBehaviorTree(BTAsset);
        //UE_LOG(LogTemp, Log, TEXT("[AI] InitFromDataAsset - BT 실행: %s"), *InBT->GetName());
    }
}

void ASMMonsterAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // Perception 감지는 로그용으로만 사용
    if (Stimulus.WasSuccessfullySensed())
    {
        // 감지해도 Blackboard는 건드리지 않음
        // CheckAttackRange()의 FindBestTarget()이 우선순위대로 처리함
    }
    else
    {
        // 시야에서 벗어났을 때도 CheckAttackRange()가 알아서 BaseCamp로 복귀
    }
}

void ASMMonsterAIController::UpdateTargetAndTryAttack()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    // ── 1단계: 이동 목표는 항상 BaseCamp로 유지 ──
    AActor* BaseCamp = FindBaseCamp();
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        // BaseCamp가 파괴됐으면 nullptr이 들어가서 BT MoveTo가 자연스럽게 실패
        BB->SetValueAsObject(FName("TargetActor"), BaseCamp);
    }

    // ── 2단계: AttackRange 안에 공격 가능한 대상이 있는지 확인 ──
    AActor* AttackTarget = FindAttackableTarget();
    
    if (!AttackTarget)
    {
        // 공격할 게 없음 → IsAttacking 해제, 포커스 해제, BT가 BaseCamp로 이동
        CurrentTargetType = EMonsterAttackTargetType::BaseCamp;
        CurrentAttackTarget = nullptr;
        if (UBlackboardComponent* BB = GetBlackboardComponent())
        {
            BB->SetValueAsBool(FName("IsAttacking"), false);
        }
        ClearFocus(EAIFocusPriority::Gameplay);
        return;
    }

    // 공격 대상 있음 → IsAttacking 설정, 타겟 바라보기
    CurrentAttackTarget = AttackTarget;
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsBool(FName("IsAttacking"), true);
    }
    SetFocus(AttackTarget, EAIFocusPriority::Gameplay);

    // ── 3단계: 공격 대상 있음 → 어빌리티 실행 ──
    UAbilitySystemComponent* ASC = nullptr;
    if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MyPawn))
    {
        ASC = ASCInterface->GetAbilitySystemComponent();
    }
    if (!ASC) return;

    const TArray<FGameplayAbilitySpec>& AllSpecs = ASC->GetActivatableAbilities();
    if (AllSpecs.Num() == 0)
    {
        return;
    }

    const FGameplayAbilitySpec& Spec = AllSpecs[0];
    bool bActivated = ASC->TryActivateAbility(Spec.Handle);
    //UE_LOG(LogTemp, Warning, TEXT("[AI] TryActivateAbility 결과: %s"), bActivated ? TEXT("성공") : TEXT("실패"));
}

AActor* ASMMonsterAIController::FindAttackableTarget()
{
    // 우선순위 1: 범위 내 건물
    if (AActor* Building = FindBuildingInRange())
    {
        CurrentTargetType = EMonsterAttackTargetType::Building;
        return Building;
    }

    // 우선순위 2: 범위 내 플레이어
    if (AActor* Player = FindNearestPlayerInRange())
    {
        CurrentTargetType = EMonsterAttackTargetType::Player;
        return Player;
    }

    // 우선순위 3: 범위 내 BaseCamp
    APawn* MyPawn = GetPawn();
    if (MyPawn)
    {
        AActor* BaseCamp = FindBaseCamp();
        if (BaseCamp)
        {
            float Dist = FVector::Dist(MyPawn->GetActorLocation(), BaseCamp->GetActorLocation());
            if (Dist <= AttackRange)
            {
                CurrentTargetType = EMonsterAttackTargetType::BaseCamp;
                return BaseCamp;
            }
        }
    }

    return nullptr;
}

AActor* ASMMonsterAIController::FindBuildingInRange()
{
    // AttackRange 이내에서 가장 가까운 건물 반환
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    const FVector MyLocation = MyPawn->GetActorLocation();
    float ClosestDist = AttackRange;
    AActor* BestTarget = nullptr;

    TArray<AActor*> FoundBuildings;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMBaseBuilding::StaticClass(), FoundBuildings);

    for (AActor* Actor : FoundBuildings)
    {
        ASMBaseBuilding* Building = Cast<ASMBaseBuilding>(Actor);
        if (!Building) continue;

        //파괴 불가능 혹은 이미 불가능한 건물 스킵
        if (!Building->GetIsDestructible()) continue;
        if (Building->GetCurrentHealth() <= 0.f) continue;
        if (Building->GetIsBeingMoved()) continue;
        
        float Dist = FVector::Dist(MyLocation, Building->GetActorLocation());
        if (Dist <= ClosestDist)
        {
            ClosestDist = Dist;
            BestTarget = Building;
        }
    }

    return BestTarget;
}

AActor* ASMMonsterAIController::FindNearestPlayerInRange()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    float ClosestDist = AttackRange;
    AActor* BestTarget = nullptr;

    // HP 체크 - 살아있으면 true
    auto IsPlayerAlive = [](APawn* PlayerPawn) -> bool
        {
            IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PlayerPawn);
            if (!ASCInterface) return true; // ASC 없으면 살아있다고 간주

            UAbilitySystemComponent* PlayerASC = ASCInterface->GetAbilitySystemComponent();
            if (!PlayerASC) return true;

            for (const UAttributeSet* AS : PlayerASC->GetSpawnedAttributes())
            {
                if (!AS) continue;
                for (TFieldIterator<FProperty> PropIt(AS->GetClass()); PropIt; ++PropIt)
                {
                    if (PropIt->GetName() == TEXT("Health"))
                    {
                        FGameplayAttribute HealthAttr(*PropIt);
                        float HP = PlayerASC->GetNumericAttribute(HealthAttr);
                        return HP > 0.f;
                    }
                }
            }
            return true; // Health 어트리뷰트 못 찾으면 살아있다고 간주
        };

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        APawn* PlayerPawn = PC->GetPawn();
        if (!PlayerPawn) continue;

        if (!IsPlayerAlive(PlayerPawn)) continue;

        float Dist = FVector::Dist(MyPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
        if (Dist <= ClosestDist)
        {
            ClosestDist = Dist;
            BestTarget = PlayerPawn;
        }
    }

    return BestTarget;
}

AActor* ASMMonsterAIController::FindBaseCamp()
{
    APawn* MyPawn = GetPawn();

    TArray<AActor*> FoundCamps;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASMBaseCampActor::StaticClass(), FoundCamps);

    AActor* NearestCamp = nullptr;
    float NearestDist = FLT_MAX;

    for (AActor* Camp : FoundCamps)
    {
        if (!IsValid(Camp)) continue;

        if (ASMBaseCampActor* CampActor = Cast<ASMBaseCampActor>(Camp))
        {
            if (CampActor->GetCurrentHealth() <= 0.f) continue;
        }

        float Dist = MyPawn
            ? FVector::Dist(MyPawn->GetActorLocation(), Camp->GetActorLocation())
            : 0.f;

        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            NearestCamp = Camp;
        }
    }

    return NearestCamp;
}
