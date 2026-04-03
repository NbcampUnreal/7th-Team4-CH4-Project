#include "Enemy/SMMonsterAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Kismet/GameplayStatics.h"

ASMMonsterAIController::ASMMonsterAIController()
{
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));

    // 2. 델리게이트에 내가 만든 함수를 등록 (바인딩)
    // "이벤트가 발생하면 내 OnTargetDetected 함수를 실행해줘!"라는 뜻입니다.
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ASMMonsterAIController::OnTargetDetected);
    
}

void ASMMonsterAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    //나중에 behaviorTree
    //MoveToLocation(FVector::ZeroVector);
    // 블랙보드 에셋이 유효한지 확인 후 초기화
    if (BBAsset && BTAsset)
    {
        if (UseBlackboard(BBAsset, BlackboardComp))
        {
            // 2. 월드에서 플레이어 캐릭터를 찾아 블랙보드에 강제 주입
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            //GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("UseBlackboard!")));
            if (PlayerPawn)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Target find!")));
                Blackboard->SetValueAsObject(FName("TargetActor"), PlayerPawn);
            }

            // 3. 비헤이비어 트리 실행
            RunBehaviorTree(BTAsset);
            if (Blackboard->GetKeyID(FName("TargetActor")) == FBlackboard::InvalidKey)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("블랙보드에 'TargetActor' 키가 존재하지 않습니다!")));
            }
            else {
                GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("블랙보드에 'TargetActor' 키가 존재함!")));

            }
        }
    }
}

void ASMMonsterAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // 여기서 타겟이 플레이어인지 구조물인지 판단하여 블랙보드 갱신
    if (Actor && Stimulus.WasSuccessfullySensed())
    {
        if (Actor->ActorHasTag(FName("Player")))
        {
            GetBlackboardComponent()->SetValueAsObject(FName("TargetActor"), Actor);
            // ... 추가 로직
        }
    }
}
