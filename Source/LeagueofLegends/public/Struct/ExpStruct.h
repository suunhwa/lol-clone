// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ExpStruct.generated.h"


USTRUCT(BlueprintType)
struct FPlayerLevelExpRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FPlayerLevelExpRow()
		: Level(0), RequiredXP(0), TotalXP(0) {}
	
	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | PlayerLevel")
	int32 Level;
	
	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | PlayerLevel")
	float RequiredXP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | PlayerLevel")
	float TotalXP;
};

	
USTRUCT(BlueprintType)
struct FUnitRewardExpRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FUnitRewardExpRow()
		: BaseXP(0), GrowthPerMinute(0), MaxXP(0)
		, ExpRadius(0), SharingMultiplier(0), UnitType(TEXT("")) {}
	
	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	float BaseXP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	float GrowthPerMinute;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	float MaxXP;
	
	// DataTable BlueSection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	float ExpRadius;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	float SharingMultiplier;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | UnitReward")
	FString UnitType;
};

USTRUCT(BlueprintType)
struct FChampionKillExpRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FChampionKillExpRow()
		: EnemyLevel(0), RewardXP(0) {}
	
	// DataTalbe Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | ChampionKill")
	int32 EnemyLevel;
	
	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exp | ChampionKill")
	float RewardXP;
	
};
