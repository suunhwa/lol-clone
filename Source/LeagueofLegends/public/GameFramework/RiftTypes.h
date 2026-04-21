#pragma once

#include "CoreMinimal.h"
#include "RiftTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
	InGame            UMETA(DisplayName = "In Game"),
	GameOver          UMETA(DisplayName = "Game Over"),
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None UMETA(DisplayName = "None"),
	Blue UMETA(DisplayName = "Blue"),
	Red  UMETA(DisplayName = "Red"),
};

// 필요한가?
UENUM(BlueprintType)
enum class ELane : uint8
{
	None    UMETA(DisplayName = "None"),
	Top     UMETA(DisplayName = "Top"),
	Jungle  UMETA(DisplayName = "Jungle"),
	Mid     UMETA(DisplayName = "Mid"),
	Bot     UMETA(DisplayName = "Bot"),
	Support UMETA(DisplayName = "Support"),
};

UENUM(BlueprintType)
enum class ESummonerSpell : uint8
{
	None     UMETA(DisplayName = "None"),
	Flash    UMETA(DisplayName = "Flash"),
	Ignite   UMETA(DisplayName = "Ignite"),
	Exhaust  UMETA(DisplayName = "Exhaust"),
	Heal     UMETA(DisplayName = "Heal"),
	Ghost    UMETA(DisplayName = "Ghost"),
	Cleanse  UMETA(DisplayName = "Cleanse"),
};
