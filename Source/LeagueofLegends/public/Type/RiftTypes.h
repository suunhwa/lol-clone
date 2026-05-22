#pragma once

#include "CoreMinimal.h"
#include "RiftTypes.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Lobby UMETA(DisplayName = "Lobby"),
	ChampionSelect UMETA(DisplayName = "Champion Select"),
	InGame UMETA(DisplayName = "In Game"),
	GameOver UMETA(DisplayName = "Game Over"),
};

UENUM(BlueprintType)
enum class EMatchMode : uint8
{
	SummonersRift UMETA(DisplayName = "소환사의 협곡"),
	DodgeBall UMETA(DisplayName = "문도피구"),
	EventMode UMETA(DisplayName = "이벤트 모드"),
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None UMETA(DisplayName = "None"),
	Blue UMETA(DisplayName = "Blue"),
	Red UMETA(DisplayName = "Red"),
};

// 필요한가?
UENUM(BlueprintType)
enum class ELane : uint8
{
	None UMETA(DisplayName = "None"),
	Top UMETA(DisplayName = "Top"),
	Jungle UMETA(DisplayName = "Jungle"),
	Mid UMETA(DisplayName = "Mid"),
	Bot UMETA(DisplayName = "Bot"),
	Support UMETA(DisplayName = "Support"),
};

UENUM(BlueprintType)
enum class ESummonerSpell : uint8
{
	None UMETA(DisplayName = "None"),
	Flash UMETA(DisplayName = "Flash"),
	Ignite UMETA(DisplayName = "Ignite"),
	Exhaust UMETA(DisplayName = "Exhaust"),
	Heal UMETA(DisplayName = "Heal"),
	Ghost UMETA(DisplayName = "Ghost"),
	Cleanse UMETA(DisplayName = "Cleanse"),
};

UENUM(BlueprintType)
enum class EUnitType : uint8
{
	Champion UMETA(DisplayName = "Champion"),
	Minion UMETA(DisplayName = "Minion"),
	Tower UMETA(DisplayName = "Tower"),
	Inhibitor UMETA(DisplayName = "Inhibitor"),
	Nexus UMETA(DisplayName = "Nexus"),
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical UMETA(DisplayName = "Physical"),
	Magical UMETA(DisplayName = "Magical"),
	TrueDamage UMETA(DisplayName = "True"),
};

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Moving UMETA(DisplayName = "Moving"),
	BasicAttacking UMETA(DisplayName = "BasicAttacking"),
	CastingSkill UMETA(DisplayName = "CastingSkill"),
	Hit UMETA(DisplayName = "Hit"),
	Dead UMETA(DisplayName = "Dead"),
	Recalling UMETA(DisplayName = "Recalling"),
};

UENUM(BlueprintType)
enum class EStatusEffect : uint8
{
	Stun UMETA(DisplayName = "Stun"),
	Root UMETA(DisplayName = "Root"),
	Slow UMETA(DisplayName = "Slow"),
	Silence UMETA(DisplayName = "Silence"),
	Knockup UMETA(DisplayName = "Knockup"),
};
