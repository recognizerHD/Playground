// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedEnum.h"
#include "EnumGameSessions.generated.h"


UENUM(BlueprintType)
enum class ERisk : uint8
{
	RE_Easy UMETA(DisplayName="Easy"),
	RE_Moderate UMETA(DisplayName="Moderate"),
	RE_Hard UMETA(DisplayName="Hard"),
	RE_Professional UMETA(DisplayName="Professional"),
	RE_Elite UMETA(DisplayName="Elite"),
};

UENUM(BlueprintType)
enum class EOperationSize : uint8
{
	OSE_Short UMETA(DisplayName="Short"), // 3 Objectives
	OSE_Medium UMETA(DisplayName="Medium"), // 5 Objectives
	OSE_Long UMETA(DisplayName="Long"), // 7 Objectives
	OSE_Enduring UMETA(DisplayName="Enduring"), // 9 Objectives
};

UENUM(BlueprintType)
enum class EGameModifiers : uint8
{
	GME_Hunter UMETA(DisplayName="Hunter"), // A bounty hunter or two hunt the players. This should be the most elite kind of fight. 
	GME_Clash UMETA(DisplayName="Clash"), // Two factions are on the map and there are places where they are fighting each other.
	GME_Foggy UMETA(DisplayName="Foggy"), // Lots of Fog
	GME_LowAmmo UMETA(DisplayName="Low Ammo"), // Ammo reserves are much more limited.
	GME_LowGravity UMETA(DisplayName="Low Gravity"), // Gravity is lower than normal.
	GME_Jammers UMETA(DisplayName="Jammers"), // All objects are points of interest with no more details. Radar is glitchy.
	GME_BadWeather UMETA(DisplayName="Bad Weather"), // There's weather.
};
/**
 * 
 */
UCLASS()
class PLAYGROUND_API UEnumGameSessions : public UUserDefinedEnum
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERisk RiskEnum;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EOperationSize OperationSizeEnum;
};
