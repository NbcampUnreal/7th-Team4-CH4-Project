#include "GAS/Abilities/GA_MonsterAttackBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"         // IAbilitySystemInterface
#include "GameplayEffect.h"
#include "Enemy/SMMonsterBase.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "Character/SMPlayerCharacter.h"  
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Enemy/SMMonsterAIController.h"
#include "Building/SMBaseCampActor.h"

UGA_MonsterAttackBase::UGA_MonsterAttackBase()
{    // 서버에서만 실행, 클라이언트는 복제로 받음
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MonsterAttackBase::ActivateAbility(

    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!AttackMontage)
    {
        //UE_LOG(LogTemp, Warning, TEXT("[Attack] AttackMontage 없음! 종료")); // ★ 임시 추가

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    // 공격 시작 시 State.Attacking 태그 부착
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->AddLooseGameplayTags(AttackingTags);
    }
    // 1. 몽타주 재생
    UAbilityTask_PlayMontageAndWait* MontageTask =
        UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this, FName("Attack"), AttackMontage, 1.0f);

    MontageTask->OnCompleted.AddDynamic(this, &UGA_MonsterAttackBase::OnMontageCompleted);
    MontageTask->OnCancelled.AddDynamic(this, &UGA_MonsterAttackBase::OnMontageCancelled);
    MontageTask->OnInterrupted.AddDynamic(this, &UGA_MonsterAttackBase::OnMontageCancelled);
    MontageTask->ReadyForActivation();

    // 2. AnimNotify가 보낼 HitEventTag 대기
    UAbilityTask_WaitGameplayEvent* WaitEventTask =
        UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag);

    WaitEventTask->EventReceived.AddDynamic(this, &UGA_MonsterAttackBase::OnHitEventReceived);
    WaitEventTask->ReadyForActivation();
    // 애니메이션 없이 테스트할 때는 바로 호출
    //OnHitEventReceived(FGameplayEventData());
    //TODO : Base Camp 공격 시
    //데미지 처리
}

// EndAbility에서 태그 반드시 해제
void UGA_MonsterAttackBase::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->RemoveLooseGameplayTags(AttackingTags);
    }

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
void UGA_MonsterAttackBase::OnHitEventReceived(FGameplayEventData Payload)
{
    // 서버에서만 데미지 적용
    if (!GetActorInfo().IsNetAuthority())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AActor* SourceActor = GetAvatarActorFromActorInfo();
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

    FHitResult HitResult;
    bool bHit = PerformHitCheck(HitResult);

    if (bHit && HitResult.GetActor())
    {
        UAbilitySystemComponent* TargetASC = nullptr;
        if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(HitResult.GetActor()))
        {
            TargetASC = ASCInterface->GetAbilitySystemComponent();
        }
        // ★ 디버그 로그 추가
        UE_LOG(LogTemp, Warning, TEXT("[Attack] TargetASC: %s, DamageEffectClass: %s"),
            TargetASC ? TEXT("Valid") : TEXT("NULL"),
            DamageEffectClass ? TEXT("Valid") : TEXT("NULL"));

        if (TargetASC && SourceASC && DamageEffectClass)
        {
            FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
            EffectContext.AddHitResult(HitResult);

            float DamageAmount = GetMonsterAttackPower();

            FGameplayEffectSpecHandle SpecHandle =
                SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);

            if (SpecHandle.IsValid())
            {
                SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                    FGameplayTag::RequestGameplayTag("Data.Damage.Amount"), -DamageAmount);

                SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                // ★ 디버그 로그 추가
                bool bApplied = SourceASC->ApplyGameplayEffectSpecToTarget(
                    *SpecHandle.Data.Get(), TargetASC).IsValid();

                UE_LOG(LogTemp, Warning, TEXT("[Attack] GE Apply 결과: %s"),
                    bApplied ? TEXT("SUCCESS") : TEXT("FAILED"));

                UE_LOG(LogTemp, Log, TEXT("[Attack] %s -> %s | Damage: %.0f"),
                    *SourceActor->GetName(),
                    *HitResult.GetActor()->GetName(),
                    DamageAmount);
            }
        }
    }

    // 몽타주가 끝나면 OnMontageCompleted에서 EndAbility가 호출되므로
    // 여기서는 EndAbility를 호출하지 않음
    // (히트 이벤트는 몽타주 중간에 발생하므로 몽타주 완료를 기다림)

    //EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UGA_MonsterAttackBase::PerformHitCheck(FHitResult& OutHitResult) const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return false;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return false;

    // AI Controller에서 현재 타겟 타입 읽기
    EMonsterAttackTargetType TargetType = EMonsterAttackTargetType::BaseCamp;
    if (APawn* Pawn = Cast<APawn>(AvatarActor))
    {
        if (ASMMonsterAIController* MonsterAI = Cast<ASMMonsterAIController>(Pawn->GetController()))
        {
            TargetType = MonsterAI->CurrentTargetType;
        }
    }

    const FVector Start = AvatarActor->GetActorLocation();
    const FVector End = Start + AvatarActor->GetActorForwardVector() * AttackRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(AvatarActor);

    // 타겟 타입에 따라 적절한 콜리전 채널 선택
    ECollisionChannel TraceChannel;
    switch (TargetType)
    {
    case EMonsterAttackTargetType::Player:
        TraceChannel = ECC_Pawn;
        break;
    case EMonsterAttackTargetType::BaseCamp:
        TraceChannel = BaseCampTraceChannel;  // StaticMesh 기반이므로 별도 채널
        break;
    case EMonsterAttackTargetType::Building:
        TraceChannel = BaseCampTraceChannel;  // 건물도 동일 채널 사용 (추후 분리 가능)
        break;
    default:
        TraceChannel = ECC_Pawn;
        break;
    }

    TArray<FHitResult> HitResults;
    World->SweepMultiByChannel(
        HitResults,
        Start, End,
        FQuat::Identity,
        TraceChannel,
        FCollisionShape::MakeSphere(AttackRadius),
        Params
    );

    for (FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor) continue;

        switch (TargetType)
        {
        case EMonsterAttackTargetType::Building:
            // TODO: 구조물 클래스 완성 후 Cast 대상 교체
            // if (Cast<ASMStructureBase>(HitActor)) { OutHitResult = Hit; return true; }
            break;

        case EMonsterAttackTargetType::Player:
            if (Cast<ASMPlayerCharacter>(HitActor))
            {
                OutHitResult = Hit;
                return true;
            }
            break;

        case EMonsterAttackTargetType::BaseCamp:
            if (Cast<ASMBaseCampActor>(HitActor))
            {
                OutHitResult = Hit;
                return true;
            }
            break;

        default:
            break;
        }
    }

    return false;
}

float UGA_MonsterAttackBase::GetMonsterAttackPower() const
{
    ASMMonsterBase* Monster = Cast<ASMMonsterBase>(GetAvatarActorFromActorInfo());
    if (Monster && Monster->MonsterAttributeSet)
    {
        return Monster->MonsterAttributeSet->GetAttackPower();
    }

    //UE_LOG(LogTemp, Warning, TEXT("[Attack] MonsterAttributeSet을 찾지 못해 기본 공격력(10)을 사용합니다."));
    return 10.0f;
}
void UGA_MonsterAttackBase::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MonsterAttackBase::OnMontageCancelled()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}