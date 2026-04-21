// SMSkillTurret.h

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "SMSkillTurret.generated.h"

class ASMASkillProjectile;
class UNiagaraComponent;
struct FGameplayEffectSpecHandle;

UCLASS()
class SAGOMAGIC_API ASMSkillTurret : public AActor
{
	GENERATED_BODY()

public:
	ASMSkillTurret();
	
	void InitTurret(
		FGameplayEffectSpecHandle InSpecHandle,
		FGameplayEffectSpecHandle InSplashSpecHandle,
		AActor* InInstigatorActor,
		float InDuration,
		float InRangeCm,
		float InFireInterval,
		int32 InSkillLevel);

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayFireSound();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UNiagaraComponent> TurretEffect;
	
	/** 발사할 투사체 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	TSubclassOf<ASMASkillProjectile> ProjectileClass;
	
	/** 스플래쉬 반경 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float SplashRadiusCm = 300.0f;

	/** 타워의 기본 발사 간격 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float DefaultFireInterval = 1.0f;

	/** 두 번째 발사 딜레이 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Turret")
	float DualFireDelay = 0.1f;
	
private:
	void FireAtNearestEnemy();
	void SpawnProjectile(const FVector& Direction);
	void OnDurationExpired();
	
	FGameplayEffectSpecHandle DamageSpecHandle;
	FGameplayEffectSpecHandle SplashSpecHandle;
	
	TWeakObjectPtr<AActor> InstigatorActor;
	
	float RangeCm = 1000.0f;
	int32 SkillLevel = 1;
	
	FTimerHandle DurationEndHandle;
	FTimerHandle FireTimerHandle;
	
	// 두 번째 발사용 타이머
	FTimerHandle SecondShotTimerHandle;
	
	// 두 번째 발사용 방향 캐시
	FVector CachedFireDirection;
	
	void FireSecondShot();
};
