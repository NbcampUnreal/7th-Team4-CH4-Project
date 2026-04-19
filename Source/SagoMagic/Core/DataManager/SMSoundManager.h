#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/SMSoundData.h"
#include "SMSoundManager.generated.h"

enum class ESMSoundCategory : uint8;
/**
 * [DataTable 설정]
 *   컬럼 설명:
 *     Row Name           → 내부 식별자 (자유롭게 작성)
 *     Sound Name         → 코드에서 호출할 ID (이 값으로 검색)
 *     Sound Asset        → 재생할 사운드 에셋 (.wav / SoundCue)
 *     Volume Multiplier  → 기본 볼륨 배율 (1.0 = 100%)
 *     Category           → BGM / SFX / UI (SoundClass 자동 적용)
 *     Concurrency        → CC_BGM 또는 CC_SFX 할당
 *
 *   예시:
 *     Row Name  | Sound Name    | Sound Asset    | Volume | Category | Concurrency
 *     MainBGM   | MainBGM       | S_MainBGM      | 1.0    | BGM      | CC_BGM
 *     AttackSFX | MonsterAttack | S_MonsterAtk   | 0.8    | SFX      | CC_SFX
 *     ButtonClick| UI_Click     | S_Click        | 1.0    | UI       | CC_SFX
 *
 * ──사용법 ────────────────────────────────────────────────────
 *
 *   USMSoundManager* SM = USMSoundManager::Get(this);
 *   if (!SM) return;
 *
 *   // BGM 재생 (페이드인 0.5초, 페이드아웃 0.5초)
 *   SM->PlayBGM(TEXT("MainBGM"));
 *   SM->PlayBGM(TEXT("MainBGM"), 1.0f, 0.5f);
 *   SM->StopBGM(0.5f);
 *
 *   // SFX - 위치 기반 3D 사운드
 *   SM->PlaySoundAtLocation(TEXT("MonsterAttack"), GetActorLocation());
 *
 *   // SFX - 액터에 붙어서 따라다니는 사운드
 *   SM->PlaySoundAttached(TEXT("MonsterAttack"), GetMesh());
 *
 * ──볼륨 저장/로드 ────────────────────────────────────────────
 *
 *   SetMasterVolume / SetBGMVolume / SetSFXVolume 호출 시
 *   자동으로 GameUserSettings.ini에 저장됨
 *   게임 재시작 후 Initialize()에서 자동으로 불러와 적용됨
 *
 */
UCLASS()
class SAGOMAGIC_API USMSoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	static USMSoundManager* Get(const UObject* WorldContext);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	UFUNCTION(BlueprintCallable)
	void PlaySoundAtLocation(FName SoundID, FVector Location);
	
	UFUNCTION(BlueprintCallable)
	void PlaySoundAttached(FName SoundID, USceneComponent* AttachToComponent);
	
	UFUNCTION(BlueprintCallable)
	void PlayBGM(FName SoundID, float FadeInTime = 0.5f, float FadeOutTime = 0.5f, bool bRestartIfSame = false);
	
	UFUNCTION(BlueprintCallable)
	void StopBGM(float FadeOutTime = 0.5f);
	
	UFUNCTION(BlueprintCallable)
	void SetMasterVolume(float Volume);
	
	UFUNCTION(BlueprintCallable)
	void SetBGMVolume(float Volume);
	
	UFUNCTION(BlueprintCallable)
	void SetSFXVolume(float Volume);
	
	float GetMasterVolume() const { return MasterVolume; }
	float GetBGMVolume() const { return BGMVolume; }
	float GetSFXVolume() const { return SFXVolume; }
	
	void SaveAudioSettings();
	void LoadAudioSettings();
private:
	FSMSoundData* GetSoundData(FName SoundID);
	USoundClass* GetSoundClassByCategory(ESMSoundCategory Category);
	
	UPROPERTY()
	TMap<FName, FSMSoundData> SoundCache;
	
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CurrentBGMComp = nullptr;
	
	FName CurrentBGMId = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundClass> MasterSoundClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundClass> BGMSoundClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundClass> SFXSoundClass;
	
	float MasterVolume = 1.0f;
	float BGMVolume = 1.0f;
	float SFXVolume = 1.0f;
};
