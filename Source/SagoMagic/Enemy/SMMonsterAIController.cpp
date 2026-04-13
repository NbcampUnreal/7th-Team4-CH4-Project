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

            // 초기 타겟: BaseCamp
            AActor* BaseCamp = FindBaseCamp();
            if (BaseCamp)
            {
                Blackboard->SetValueAsObject(FName("TargetActor"), BaseCamp);
                CurrentTargetType = EMonsterAttackTargetType::BaseCamp;
            }

            //bool bBTStarted = RunBehaviorTree(BTAsset);
            RunBehaviorTree(BTAsset);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AI] OnPossess - BBAsset: %s / BTAsset: %s"),
            BBAsset ? TEXT("있음") : TEXT("NULL"),
            BTAsset ? TEXT("있음") : TEXT("NULL"));
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

void ASMMonsterAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    //if (Actor && Stimulus.WasSuccessfullySensed())
    //{
    //    if (Actor->ActorHasTag(FName("Player")))
    //    {
    //        //UE_LOG(LogTemp, Warning, TEXT("[AI] OnTargetDetected - 플레이어 감지! TargetActor를 %s로 덮어씀"),
    //        //    *Actor->GetName());
    //        UE_LOG(LogTemp, Warning, TEXT("[AI] OnTargetDetected - 플레이어 감지 시각: %f / Actor: %s"),
    //            GetWorld()->GetTimeSeconds(), *Actor->GetName());
    //        GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Actor);
    //    }
    //}
    //if (!Actor) return;

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

    // 타겟 우선순위 결정
    AActor* BestTarget = FindBestTarget();
    if (!BestTarget) return;

    // Blackboard에 최종 타겟 반영 (BT 이동에도 활용 가능)
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsObject(FName("TargetActor"), BestTarget);
    }
    // 타겟까지 거리 확인 — 범위 밖이면 이동만 하고 어빌리티는 실행하지 않음
    float DistToTarget = FVector::Dist(MyPawn->GetActorLocation(), BestTarget->GetActorLocation());
   
    // BaseCamp는 AttackRange 대신 별도 근접 판정 사용 가능
    // 지금은 동일한 AttackRange로 통일
    if (DistToTarget > AttackRange)
    {
        // 사거리 밖 → BT가 MoveTo로 접근 중이므로 여기서는 리턴
        return;
    }

    // ASC에서 어빌리티 실행
    UAbilitySystemComponent* ASC = nullptr;
    if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MyPawn))
    {
        ASC = ASCInterface->GetAbilitySystemComponent();
    }

    if (!ASC)
    {
        return;
    }

    const TArray<FGameplayAbilitySpec>& AllSpecs = ASC->GetActivatableAbilities();

    if (AllSpecs.Num() == 0)
    {
        return;
    }

    const FGameplayAbilitySpec& Spec = AllSpecs[0];
    ASC->TryActivateAbility(Spec.Handle);
}

AActor* ASMMonsterAIController::FindBestTarget()
{
    // 우선순위 1: 경로 위 구조물
    // TODO: 구조물 클래스 완성 후 FindBuildingOnPath()가 실제 결과를 반환함
    if (AActor* Building = FindBuildingOnPath())
    {
        CurrentTargetType = EMonsterAttackTargetType::Building;
        return Building;
    }

    // 우선순위 2: 범위 내 가장 가까운 플레이어
    if (AActor* Player = FindNearestPlayerInRange())
    {
        CurrentTargetType = EMonsterAttackTargetType::Player;
        return Player;
    }

    // 우선순위 3: BaseCamp
    if (AActor* BaseCamp = FindBaseCamp())
    {
      //UE_LOG(LogTemp, Warning, TEXT("[AI] FindBaseCamp!!"));
        CurrentTargetType = EMonsterAttackTargetType::BaseCamp;
        return BaseCamp;
    }

    // 공격 대상 없음
    CurrentTargetType = EMonsterAttackTargetType::None;
    return nullptr;
}

AActor* ASMMonsterAIController::FindBuildingOnPath()
{
    // TODO: 건축물 완성 후 아래 로직 구현
    return nullptr;
}

AActor* ASMMonsterAIController::FindNearestPlayerInRange()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return nullptr;

    float ClosestDist = PlayerDetectRadius;  // ★ AttackRange가 아닌 별도 감지 반경 사용
    AActor* BestTarget = nullptr;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        APawn* PlayerPawn = PC->GetPawn();
        if (!PlayerPawn) continue;

        // 죽은 플레이어 제외
        if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(PlayerPawn))
        {
            if (UAbilitySystemComponent* PlayerASC = ASCInterface->GetAbilitySystemComponent())
            {
                // Health Attribute를 찾아 0 이하인 플레이어 스킵
                for (const UAttributeSet* AS : PlayerASC->GetSpawnedAttributes())
                {
                    if (!AS) continue;
                    for (TFieldIterator<FProperty> PropIt(AS->GetClass()); PropIt; ++PropIt)
                    {
                        if (PropIt->GetName() == TEXT("Health"))
                        {
                            FGameplayAttribute HealthAttr(*PropIt);
                            float HP = PlayerASC->GetNumericAttribute(HealthAttr);
                            if (HP <= 0.f)
                            {
                                PlayerPawn = nullptr; // 죽은 플레이어 표시
                            }
                            goto DoneCheckHP;
                        }
                    }
                }
            }
        }
    DoneCheckHP:
        if (!PlayerPawn) continue;

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

        // HP가 0인 BaseCamp는 제외
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