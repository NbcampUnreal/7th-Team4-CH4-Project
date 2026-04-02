#pragma once

#include "CoreMinimal.h"
#include "SMInventoryCoreTypes.generated.h"


/**
 * 인벤토리 시스템 전반에서 공통으로 사용하는 기본 enum 및 마스크 구조 정의 파일
 *
 * 포함 내용:
 * - 아이템 종류 Enum
 * - 컨테이너 종류 Enum
 * - 젬 효과 종류 Enum
 * - 비트마스크 기반 그리드 모양 데이터 Struct
 *
 * 역할:
 * - 인벤토리 관련 런타임/정의 데이터의 가장 기초가 되는 공통 타입 제공
 */

/** 아이템 종류 구분 enum */
UENUM(BlueprintType)
enum class ESMItemType : uint8
{
	None	UMETA(DisplayName="None"),
	Gem		UMETA(DisplayName="Gem"),
	Skill	UMETA(DisplayName="Skill")
};

/** 인벤토리 컨테이너 종류 구분 enum */
UENUM(BlueprintType)
enum class ESMContainerType : uint8
{
	None			UMETA(DisplayName="None"),
	MainInventory	UMETA(DisplayName="MainInventory"),
	SkillInternal	UMETA(DisplayName="SkillInternal"),
	QuickSlot		UMETA(DisplayName="QuickSlot"),
	WorldDrop		UMETA(DisplayName="WorldDrop")
};

/** 젬 효과 종류 구분 enum */
UENUM(BlueprintType)
enum class ESMGemModifierType : uint8
{
	None			UMETA(DisplayName="None"),
	Effect			UMETA(DisplayName="Effect"),
	RangeOrArea		UMETA(DisplayName="RangeOrArea"),
	Cooldown		UMETA(DisplayName="Cooldown")
};

/** 비트마스크 기반 그리드 모양 데이터 */
USTRUCT(BlueprintType)
struct SAGOMAGIC_API FSMGridMaskData
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	FSMGridMaskData()
        : Width(1)
		, Height(1)
		, BitMask(TEXT("1"))
	{
	}

	/** 가로 크기 Getter */
	int32 GetWidth() const
	{
		return Width;
	}

	/** 세로 크기 Getter */
	int32 GetHeight() const
	{
		return Height;
	}

	/** 비트마스크 Getter */
	const FString& GetBitMask() const
	{
		return BitMask;
	}

	/** 가로 크기 Setter */
	void SetWidth(const int32 InputWidth)
	{
		Width = InputWidth;
	}

	/** 세로 크기 Setter */
	void SetHeight(const int32 InputHeight)
	{
		Height = InputHeight;
	}

	/** 비트마스크 Setter */
	void SetBitMask(const FString& InputBitMask)
	{
		BitMask = InputBitMask;
	}

public:
	/** 가로/세로 크기 유효성 검사 */
	bool IsValidSize() const
	{
		return Width > 0 && Height > 0;
	}

	/** 비트마스크 길이 유효성 검사 */
	bool IsValidMaskLength() const
	{
		return BitMask.Len() == (Width * Height);
	}

	/** 이진 문자열 여부 검사 */
	bool IsBinaryMask() const
	{
		for (const TCHAR c : BitMask)
		{
			if (c != TEXT('0') && c != TEXT('1'))
			{
				return false;
			}
		}

		return true;
	}

	/** 전체 마스크 데이터 유효성 검사 */
	bool IsValidMaskData() const
	{
		return IsValidSize() && IsValidMaskLength() && IsBinaryMask();
	}

public:
	/** 마스크 가로 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Mask")
	int32 Width;

	/** 마스크 세로 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Mask")
	int32 Height;

	/** 0/1 문자열 형태 마스크 데이터(아이템 형태/스킬 인벤토리 형태) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory|Mask")
	FString BitMask;
};
