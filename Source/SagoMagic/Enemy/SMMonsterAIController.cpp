#include "Enemy/SMMonsterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "Enemy/SMMonsterBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

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

            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Target find!"));
                Blackboard->SetValueAsObject(FName("TargetActor"), PlayerPawn);
            }

            RunBehaviorTree(BTAsset);
        }
    }

  
}

void ASMMonsterAIController::OnUnPossess()
{
    Super::OnUnPossess();
    GetWorldTimerManager().ClearTimer(AttackCheckTimerHandle);
}
void ASMMonsterAIController::StartAttackTimer()
{
    // 이미 타이머가 돌고 있으면 중복 실행 방지
    if (GetWorldTimerManager().IsTimerActive(AttackCheckTimerHandle)) return;

    GetWorldTimerManager().SetTimer(
        AttackCheckTimerHandle,
        this,
        &ASMMonsterAIController::CheckAttackRange,
        AttackCooldown,
        true
    );

    //UE_LOG(LogTemp, Warning, TEXT("[AI] 공격 타이머 시작. 쿨다운: %.1f초"), AttackCooldown);
}

void ASMMonsterAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor && Stimulus.WasSuccessfullySensed())
    {
        if (Actor->ActorHasTag(FName("Player")))
        {
            GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Actor);
        }
    }
}

void ASMMonsterAIController::CheckAttackRange()
{
    /*UE_LOG(LogTemp, Warning, TEXT("[AI] NetMode:%d Role:%d LocalRole:%d"),
        (int32)GetNetMode(),
        (int32)GetPawn()->GetRemoteRole(),
        (int32)GetPawn()->GetLocalRole());*/
    /*UE_LOG(LogTemp, Warning, TEXT("[AI] CheckAttackRange - NetMode: %d, HasAuthority: %s"),
        (int32)GetNetMode(),
        HasAuthority() ? TEXT("Server") : TEXT("Client"));*/

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    UAbilitySystemComponent* ASC = nullptr;
    if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(MyPawn))
    {
        ASC = ASCInterface->GetAbilitySystemComponent();
    }

    if (!ASC)
    {
        //UE_LOG(LogTemp, Warning, TEXT("[AI] Pawn:%s / ASC 없음"), *MyPawn->GetName());
        return;
    }

    const TArray<FGameplayAbilitySpec>& AllSpecs = ASC->GetActivatableAbilities();

    /*UE_LOG(LogTemp, Warning, TEXT("[AI] Pawn:%s / ASC:%p / 부여된 어빌리티 수: %d"),
        *MyPawn->GetName(),
        ASC,
        AllSpecs.Num());*/

    if (AllSpecs.Num() == 0)
    {
        return;
    }

    const FGameplayAbilitySpec& Spec = AllSpecs[0];
    ASC->TryActivateAbility(Spec.Handle);
}
