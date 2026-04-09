#include "GAS/Abilities/GA_MonsterAttackBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"         // IAbilitySystemInterface
#include "GameplayEffect.h"
#include "Enemy/SMMonsterBase.h"
#include "GAS/AttributeSets/SMMonsterAttributeSet.h"
#include "Character/SMPlayerCharacter.h"  
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

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
  /*  UE_LOG(LogTemp, Warning, TEXT("[Attack] OnHitEventReceived 호출됨. HasAuthority: %s"),
        GetActorInfo().IsNetAuthority() ? TEXT("TRUE") : TEXT("FALSE"));*/
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

        if (TargetASC && SourceASC)
        {
            FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
            EffectContext.AddHitResult(HitResult);

            float DamageAmount = GetMonsterAttackPower();

            FGameplayEffectSpecHandle SpecHandle =
                SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContext);

            if (SpecHandle.IsValid())
            {
                /*UE_LOG(LogTemp, Warning, TEXT("[Attack] SpecHandle 유효. DamageEffectClass: %s"),
                    DamageEffectClass ? *DamageEffectClass->GetName() : TEXT("NULL ← 에디터에서 할당 필요!"));*/
                SpecHandle.Data.Get()->SetSetByCallerMagnitude(
                    FGameplayTag::RequestGameplayTag("Data.Damage.Amount"), -DamageAmount);

                // 타겟 ASC의 모든 AttributeSet 중 "Health" Attribute를 찾아 읽기
                // (플레이어 코드 include 없이 문자열로 탐색)
                float HPBeforeVal = -1.f, HPAfterVal = -1.f;
                for (const UAttributeSet* AS : TargetASC->GetSpawnedAttributes())
                {
                    if (!AS) continue;
                    for (TFieldIterator<FProperty> PropIt(AS->GetClass()); PropIt; ++PropIt)
                    {
                        if (PropIt->GetName() == TEXT("Health"))
                        {
                            FGameplayAttribute HealthAttr(*PropIt);
                            HPBeforeVal = TargetASC->GetNumericAttribute(HealthAttr);

                            SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

                            HPAfterVal = TargetASC->GetNumericAttribute(HealthAttr);

                            /*UE_LOG(LogTemp, Warning,
                                TEXT("[Attack] %s -> %s | 데미지: %.0f | 플레이어 HP: %.0f -> %.0f"),
                                *SourceActor->GetName(),
                                *HitResult.GetActor()->GetName(),
                                DamageAmount,
                                HPBeforeVal,
                                HPAfterVal);*/
                            goto ApplyDone; // 중첩 루프 탈출
                        }
                    }
                }
                // Health Attribute를 못 찾은 경우 (GE는 그냥 적용)
                SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                UE_LOG(LogTemp, Warning,
                    TEXT("[Attack] %s -> %s | 데미지: %.0f | (HP Attribute를 찾지 못했습니다)"),
                    *SourceActor->GetName(),
                    *HitResult.GetActor()->GetName(),
                    DamageAmount);
            ApplyDone:;
            }
        }
        else
        {
            // 디버그: ASC를 못 찾은 경우 원인 파악용
            UE_LOG(LogTemp, Warning, TEXT("[Attack] ASC를 찾지 못했습니다. Target:%s / SourceASC:%s / TargetASC:%s"),
                *HitResult.GetActor()->GetName(),
                SourceASC ? TEXT("OK") : TEXT("NULL"),
                TargetASC ? TEXT("OK") : TEXT("NULL"));
        }
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UGA_MonsterAttackBase::PerformHitCheck(FHitResult& OutHitResult) const
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    if (!AvatarActor) return false;

    UWorld* World = AvatarActor->GetWorld();
    if (!World) return false;

    const FVector Start = AvatarActor->GetActorLocation();
    const FVector End = Start + AvatarActor->GetActorForwardVector() * AttackRange;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(AvatarActor);

    TArray<FHitResult> HitResults;
    World->SweepMultiByChannel(
        HitResults,
        Start, End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(AttackRadius),
        Params
    );

    for (FHitResult& Hit : HitResults)
    {
        // 플레이어 캐릭터인지 타입으로 확인
        if (Cast<ASMPlayerCharacter>(Hit.GetActor()))
        {
            OutHitResult = Hit;
            //UE_LOG(LogTemp, Log, TEXT("[HitCheck] 성공: %s"), *Hit.GetActor()->GetName());
            return true;
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