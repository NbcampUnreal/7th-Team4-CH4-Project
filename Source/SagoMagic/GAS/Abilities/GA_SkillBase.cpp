#include "GAS/Abilities/GA_SkillBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/Character/SMSkillTag.h"
#include "GameFramework/Character.h"
#include "Data/SMSkillData.h"
#include "GAS/Effects/GE_SkillCooldown.h"

UGA_SkillBase::UGA_SkillBase()
{
    //어빌리티 인스턴스 생성 - 단 하나
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    //어빌리티가 네트워크 어디서 실행될지 - 클라이언트
    NetExecutionPolicy =EGameplayAbilityNetExecutionPolicy::LocalPredicted;
    //인스턴스를 네트워크로 복제할지 - 복제안함
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
}

bool UGA_SkillBase::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
    //GAS의 기본조건을 자동을 체크 (Ability, ActivationBlockedTags 등)
    return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UGA_SkillBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // DT에서 스킬 수치 로드
    if (!LoadSkillStats())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // 마우스 정보 가져오기, 커서 방향 -> CurrentAimOrigin, CurrentAimDirection
    ExtractAimData(ActorInfo);

    if (!CommitAbility(Handle,ActorInfo,ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    // TODO: HasAuthority 체크 요망
    OnSkillEffect(ActorInfo);

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_SkillBase::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
    //CooldownGameplatEffectClass가 없거나 쿨다운이 0이면 패스
    if (!CooldownGameplayEffectClass || CooldownSeconds <= 0.f)
    {
        Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
        return;
    }

    //GE Spec 생성
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
    if (!SpecHandle.IsValid()) return;
    
    //SetByCaller로 쿨다운 시간 주입.
    SpecHandle.Data->SetSetByCallerMagnitude(SMSkillTag::Data_Cooldown, CooldownSeconds);

    //스킬별 쿨다운 태그 추가
    //GA에 ActivationBlockTags에 같은 태그를 등록하면 쿨다운 중 스킬 발동 불가하는 식으로
    if (CooldownTag.IsValid())
    {
        SpecHandle.Data->DynamicGrantedTags.AddTag(CooldownTag);
    }
    
    //동적으로 쿨다운을 적용시켜주는 함수
    ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
}


FString UGA_SkillBase::GetCurrentPredictionKeyStatus()
{
    //1. 현재 사용 중인 Prediction Key에 대한 번호를 가져옴
    //2. 이 키로 아직 더 예측을 더 할 수 있는가를 확인하고 물어보고 맞으면 true, 번호가 틀리거나 아니면 false를 반환
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.ToString() + " is valid for more prediction: " + (ASC->ScopedPredictionKey.IsValidForMorePrediction() ? TEXT("true") : TEXT("false"));
}

bool UGA_SkillBase::IsPredictionKeyValidForMorePrediction() const
{
    //스킬을 방동하는 순간 Prediction Key가 생성됨 이하 예측 키, 또한 Ability Prediction Window - 유효시간이 생김.
    //예측 키는 어빌리티 활성화-> 타겟 데이터 전송 -> 코스메틱 효과 등을 한번에 처리 이것이 More Prediction
    //Prediction Key 만료 - 너무 많은 행동을 하면 서버에서 Acknowledgment하고 되서 만료가 되버림
    //Prediction Key가 만료가 되면 IsValidForMorePrediction()에서 false를 반환하고
    //새로운 Prediction Key를 생성하거나 서버의 허락을 기다려야함.
    //현재 예측 키 가 추가적인 예측을 더 할 수 있는 상태인지 bool값으로 반환.
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    return ASC->ScopedPredictionKey.IsValidForMorePrediction();
}

bool UGA_SkillBase::LoadSkillStats()
{
    const FSMSkillData* Row = SkillStatRow.GetRow<FSMSkillData>(TEXT("LoadSkillStats"));
    if (!Row) return false;

    BaseDamage = Row->BaseDamage;
    RangeCm = Row->RangeCm;
    CooldownSeconds = Row->Cooldown;
    return true;
}

void UGA_SkillBase::ExtractAimData(const FGameplayAbilityActorInfo* ActorInfo)
{
    if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return;

    ACharacter* SMCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
    if (!SMCharacter) return;

    //캐릭터 위치
    CurrentAimOrigin = SMCharacter->GetActorLocation();

    //커서가 가리키는 벡터 방향 설정하기
    if (AController* Controller = SMCharacter->GetController())
    {
        CurrentAimDirection = Controller->GetControlRotation().Vector();
    }
}
