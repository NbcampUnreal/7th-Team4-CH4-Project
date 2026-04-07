// SMFunctionLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SMFunctionLibrary.generated.h"

/**
 * 커스텀 로그 출력용 클래스
 */
UCLASS()
class SAGOMAGIC_API USMFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 네트워크 모드에 따라 문자열 메시지를 화면에 출력하거나 로그에 기록합니다.
	 *
	 * 게임이 클라이언트 또는 리슨 서버 모드로 실행 중인 경우 메시지가 화면에 표시됩니다.
	 * 그렇지 않은 경우(예: 데디케이티드 서버) 엔진의 로깅 시스템을 통해 로그로 남습니다.
	 *
	 * @param WorldContextObject 월드 및 네트워크 모드를 결정하는 데 사용되는 월드 컨텍스트 오브젝트입니다. 
	 * 일반적으로 대상 월드 내에 존재하는 UObject를 사용합니다.
	 * @param InString 화면에 표시하거나 로그로 남길 문자열 메시지입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "SM|Debug", meta = (WorldContext = "WorldContextObject"))
	static void SMPrintString(
		const UObject* WorldContextObject,
		const FString& InString,
		float InTimeToDisplay = 3.0f,
		FColor InColor = FColor::Cyan);

	/**
	 * 지정된 월드 컨텍스트의 현재 네트워크 모드(NetMode)를 문자열 형태로 반환합니다.
	 *
	 * @param WorldContextObject 네트워크 모드를 결정하는 데 사용되는 월드 컨텍스트 오브젝트입니다.
	 * 일반적으로 대상 월드 내에 존재하는 UObject를 사용합니다.
	 * 
	 * @return 네트워크 모드를 나타내는 문자열입니다. 반환 가능한 값은 다음과 같습니다:
	 * - "None": WorldContextObject가 유효하지 않거나 월드를 확인할 수 없는 경우.
	 * - "Client": 네트워크 모드가 NM_Client인 경우.
	 * - "StandAlone": 네트워크 모드가 NM_Standalone인 경우.
	 * - "Server": 그 외의 모든 네트워크 모드인 경우.
	 */
	UFUNCTION(BlueprintCallable, Category = "SM|Debug", meta = (WorldContext = "WorldContextObject"))
	static FString GetNetModeString(const UObject* WorldContextObject);

	/**
	 * 지정된 액터의 로컬 및 리모트 네트워크 역할(Role)을 나타내는 포맷된 문자열을 반환합니다.
	 *
	 * 반환되는 문자열의 형식은 "LocalRole / RemoteRole"입니다.
	 *
	 * @return 액터의 로컬 및 리모트 네트워크 역할을 나타내는 문자열이거나, 액터가 유효하지 않은 경우 "None"입니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "SM|Debug")
	static FString GetRoleString(const AActor* InActor);
	
};
