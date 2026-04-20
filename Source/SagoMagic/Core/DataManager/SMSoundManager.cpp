#include "SMSoundManager.h"

#include "SagoMagic.h"
#include "Components/AudioComponent.h"
#include "Data/SMSoundData.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"

USMSoundManager* USMSoundManager::Get(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI) return nullptr;
	return GI->GetSubsystem<USMSoundManager>();
}

void USMSoundManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MasterSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SagoMagic/SoundEffects/SC_Master.SC_Master"));
	BGMSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SagoMagic/SoundEffects/SC_BGM.SC_BGM"));
	SFXSoundClass = LoadObject<USoundClass>(nullptr, TEXT("/Game/SagoMagic/SoundEffects/SC_SFX.SC_SFX"));

	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/SagoMagic/Data/DataTables/SoundData/DT_Sound.DT_Sound"));
	if (Table)
	{
		for (auto& Row : Table->GetRowMap())
		{
			FSMSoundData* Data = reinterpret_cast<FSMSoundData*>(Row.Value);
			if (Data)
				SoundCache.Add(Data->SoundName, *Data);
		}
		SM_LOG(this, LogSM, Log, TEXT("사운드 %d개 로드 완료"), SoundCache.Num());
	}
	else
	{
		SM_LOG(this, LogSM, Log, TEXT("사운드 로드 실패"));
	}
	
	LoadAudioSettings();
}

void USMSoundManager::Deinitialize()
{
	StopBGM(0.f);
	Super::Deinitialize();
}

void USMSoundManager::PlaySoundAtLocation(FName SoundID, FVector Location)
{
	if (!GEngine || !GEngine->GetMainAudioDevice()) return;
	
	FSMSoundData* Data = GetSoundData(SoundID);
	if (!Data || !Data->SoundAsset) return;
	
	Data->SoundAsset->SoundClassObject = GetSoundClassByCategory(Data->Category);
	
	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(),
		Data->SoundAsset,
		Location,
		FRotator::ZeroRotator,
		Data->VolumeMultiplier,
		1.f,
		0.f,
		nullptr,
		Data->ConcurrencySettings
	);
}

void USMSoundManager::PlaySoundAttached(FName SoundID, USceneComponent* AttachToComponent)
{
	if (!GEngine || !GEngine->GetMainAudioDevice()) return;
	
	FSMSoundData* Data = GetSoundData(SoundID);
	if (!Data || !Data->SoundAsset) return;
	
	Data->SoundAsset->SoundClassObject = GetSoundClassByCategory(Data->Category);
	UGameplayStatics::SpawnSoundAttached(
		Data->SoundAsset,
		AttachToComponent,
		NAME_None,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		true,
		Data->VolumeMultiplier,
		1.f,
		0.f,
		nullptr,
		Data->ConcurrencySettings
	);
	
}

void USMSoundManager::PlayBGM(FName SoundID, float FadeInTime, float FadeOutTime, bool bRestartIfSame)
{
	if (!GEngine || !GEngine->GetMainAudioDevice()) return;
	
	if (!bRestartIfSame && CurrentBGMComp
		&& CurrentBGMId == SoundID
		&& CurrentBGMComp->IsPlaying())
		return;
	StopBGM(FadeOutTime);
	
	FSMSoundData* Data = GetSoundData(SoundID);
	if (!Data || !Data->SoundAsset) return;
	
	Data->SoundAsset->SoundClassObject = BGMSoundClass;
	
	UAudioComponent* NewComp = UGameplayStatics::SpawnSound2D(
		GetWorld(), Data->SoundAsset,
		Data->VolumeMultiplier,
		1.0f, 0.0f, Data->ConcurrencySettings, true);
	if (!NewComp) return;
	
	NewComp->bIsUISound = true;
	NewComp->bAutoDestroy = false;
	NewComp->FadeIn(FadeInTime, Data->VolumeMultiplier);
	
	CurrentBGMComp = NewComp;
	CurrentBGMId = SoundID;
}

void USMSoundManager::PlaySoundUI(FName SoundID)
{
	if (!GEngine || !GEngine->GetMainAudioDevice()) return;
	
	FSMSoundData* Data = GetSoundData(SoundID);
	if (!Data || !Data->SoundAsset) return;
	
	Data->SoundAsset->SoundClassObject = GetSoundClassByCategory(Data->Category);
	UGameplayStatics::SpawnSound2D(
	GetWorld(),
	Data->SoundAsset,
	Data->VolumeMultiplier,
	1.f,
	0.f,
	Data->ConcurrencySettings
	);
}

void USMSoundManager::StopBGM(float FadeOutTime)
{
	if (!GEngine || !GEngine->GetMainAudioDevice()) return;
	
	if (!CurrentBGMComp) return;
	
	if (FadeOutTime > 0.f)
	{
		CurrentBGMComp->bAutoDestroy = true;
		CurrentBGMComp->FadeOut(FadeOutTime, 0.f);
	}
	else
	{
		CurrentBGMComp->Stop();
		CurrentBGMComp->DestroyComponent();
	}
	CurrentBGMComp = nullptr;
	CurrentBGMId = NAME_None;
}

void USMSoundManager::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (MasterSoundClass)
		MasterSoundClass->Properties.Volume = MasterVolume;
	SaveAudioSettings();
}

void USMSoundManager::SetBGMVolume(float Volume)
{
	BGMVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (BGMSoundClass)
		BGMSoundClass->Properties.Volume = BGMVolume;
	SaveAudioSettings();
}

void USMSoundManager::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (SFXSoundClass)
		SFXSoundClass->Properties.Volume = SFXVolume;
	SaveAudioSettings();
}

void USMSoundManager::SaveAudioSettings()
{
	if (!GConfig) return;
	const FString Section = TEXT("/Script/SagoMagic.AudioSettings");
	GConfig->SetFloat(*Section, TEXT("MasterVolume"), MasterVolume, GGameUserSettingsIni);
	GConfig->SetFloat(*Section, TEXT("BGMVolume"), BGMVolume, GGameUserSettingsIni);
	GConfig->SetFloat(*Section, TEXT("SFXVolume"), SFXVolume, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void USMSoundManager::LoadAudioSettings()
{
	if (!GConfig) return;
	const FString Section = TEXT("/Script/SagoMagic.AudioSettings");
	GConfig->GetFloat(*Section, TEXT("MasterVolume"), MasterVolume, GGameUserSettingsIni);
	GConfig->GetFloat(*Section, TEXT("BGMVolume"), BGMVolume, GGameUserSettingsIni);
	GConfig->GetFloat(*Section, TEXT("SFXVolume"), SFXVolume, GGameUserSettingsIni);
	
	if (MasterSoundClass)
		MasterSoundClass->Properties.Volume = MasterVolume;
	if (BGMSoundClass)
		BGMSoundClass->Properties.Volume = BGMVolume;
	if (SFXSoundClass)
		SFXSoundClass->Properties.Volume = SFXVolume;
}

FSMSoundData* USMSoundManager::GetSoundData(FName SoundID)
{
	FSMSoundData* Found = SoundCache.Find(SoundID);
	if (!Found)
		UE_LOG(LogTemp, Warning, TEXT("[SoundManager] %s 없음"), *SoundID.ToString());
	return Found;
}

USoundClass* USMSoundManager::GetSoundClassByCategory(ESMSoundCategory Category)
{
	switch (Category)
	{
	case ESMSoundCategory::BGM: return BGMSoundClass;
	case ESMSoundCategory::SFX: return SFXSoundClass;
	default: return SFXSoundClass;
	}
}

UAudioComponent* USMSoundManager::PlaySoundLoopAttached(FName SoundID, USceneComponent* AttachToComponent)
{
	// 오디오 디바이스 검사 (데디 서버는 디바이스 없음 → nullptr 반환)
	if (!GEngine || !GEngine->GetMainAudioDevice()) return nullptr;

	// Attach 타겟 유효성 검사
	if (IsValid(AttachToComponent) == false) return nullptr;

	// DT 조회 및 에셋 유효성 검사
	FSMSoundData* Data = GetSoundData(SoundID);
	if (!Data || !Data->SoundAsset) return nullptr;

	// 카테고리에 맞는 SoundClass 적용 (볼륨 일괄 제어용)
	Data->SoundAsset->SoundClassObject = GetSoundClassByCategory(Data->Category);

	// bAutoDestroy=false → 호출자가 StopSoundLoop로 수명 관리
	UAudioComponent* LoopComp = UGameplayStatics::SpawnSoundAttached(
		Data->SoundAsset,
		AttachToComponent,
		NAME_None,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		true,                   
		Data->VolumeMultiplier,
		1.f,
		0.f,
		nullptr,
		Data->ConcurrencySettings
	);

	if (IsValid(LoopComp) == false) return nullptr;

	LoopComp->bAutoDestroy = false;
	return LoopComp;
}

void USMSoundManager::StopSoundLoop(UAudioComponent* LoopComponent, float FadeOutTime)
{
	// 컴포넌트 유효성 검사
	if (IsValid(LoopComponent) == false) return;

	// 페이드아웃 + 자동 파괴 (StopBGM과 동일 패턴)
	if (FadeOutTime > 0.f)
	{
		LoopComponent->bAutoDestroy = true;
		LoopComponent->FadeOut(FadeOutTime, 0.f);
	}
	else
	{
		LoopComponent->Stop();
		LoopComponent->DestroyComponent();
	}
}
