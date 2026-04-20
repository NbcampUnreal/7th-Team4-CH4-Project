// SagoMagic.cpp

#include "SagoMagic.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Modules/ModuleManager.h"

namespace
{
	constexpr int32 SteamQueryPortOffset = 27015 - 7777;

	void ConfigureSteamDedicatedServer()
	{
		if (!IsRunningDedicatedServer())
		{
			return;
		}

		int32 GamePort = 0;
		if (!FParse::Value(FCommandLine::Get(), TEXT("Port="), GamePort))
		{
			GConfig->GetInt(TEXT("URL"), TEXT("Port"), GamePort, GEngineIni);
		}

		if (GamePort <= 0)
		{
			GamePort = 7777;
		}

		int32 QueryPort = 0;
		const bool bHasQueryPortOverride = FParse::Value(FCommandLine::Get(), TEXT("QueryPort="), QueryPort);
		if (!bHasQueryPortOverride)
		{
			QueryPort = FMath::Clamp(GamePort + SteamQueryPortOffset, 1024, 65535);
			GConfig->SetInt(TEXT("OnlineSubsystemSteam"), TEXT("GameServerQueryPort"), QueryPort, GEngineIni);
		}

		FString GameVersion;
		if (!GConfig->GetString(TEXT("OnlineSubsystemSteam"), TEXT("GameVersion"), GameVersion, GEngineIni) || GameVersion.IsEmpty())
		{
			if (!GConfig->GetString(TEXT("/Script/EngineSettings.GeneralProjectSettings"), TEXT("ProjectVersion"), GameVersion, GGameIni) || GameVersion.IsEmpty())
			{
				GameVersion = TEXT("1.0.0.0");
			}

			GConfig->SetString(TEXT("OnlineSubsystemSteam"), TEXT("GameVersion"), *GameVersion, GEngineIni);
		}

		UE_LOG(
			LogSM,
			Log,
			TEXT("[SteamConfig] Dedicated server config prepared. GamePort=%d QueryPort=%d GameVersion=%s"),
			GamePort,
			QueryPort,
			*GameVersion);
	}
}

class FSagoMagicModule final : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		ConfigureSteamDedicatedServer();
		FDefaultGameModuleImpl::StartupModule();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FSagoMagicModule, SagoMagic, "SagoMagic");

#pragma region NetLogging

DEFINE_LOG_CATEGORY(LogSM)

# pragma endregion
