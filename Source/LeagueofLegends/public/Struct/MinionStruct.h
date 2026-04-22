#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MinionStruct.generated.h"

// MinionBase 시트용 
USTRUCT(BlueprintType)
struct FMinionBaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMinionBaseRow()
		: MinionID(0), MoveSpeed(0.f), AtkRange(0.f), AtkSpeed(0.f), ProjSpeed(0.f)
		  , Armor(0), MR(0), Collision(0.f), Is_Siege(false), Is_Super(false)
		  , Tower_DR(0.f), Name_KR(TEXT("")), Name_EN(TEXT("")) {}

	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion")
	int32 MinionID;

	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float AtkRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float AtkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float ProjSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	int32 Armor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	int32 MR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float Collision;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	bool Is_Siege;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	bool Is_Super;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	float Tower_DR;
	
	// DataTable Red Setion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	FString Name_KR;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Base")
	FString Name_EN;
};

// MinionGrowth 시트용 
USTRUCT(BlueprintType)
struct FMinionGrowthRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMinionGrowthRow() 
		: Minion_ID(0), Base_HP(0.f), HP_Up(0.f), Base_AD(0.f), AD_Up(0.f)
		, Base_Gold(0), Gold_Up(0.f), Interval(0.f), Max_Cycle(0) {}
	
	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	int32 Minion_ID;

	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	float Base_HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	float HP_Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	float Base_AD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	float AD_Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	int32 Base_Gold; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	int32 Gold_Up;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	float Interval;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Growth")
	int32 Max_Cycle;
};

// MinionWave 시트용 
USTRUCT(BlueprintType)
struct FMinionWaveRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMinionWaveRow() 
		: Start_Time(0), End_Time(0), Spawn_Interval(0), Siege_Cycle(0)
		, Normal_Spawn_List(TEXT("")), Special_Spawn_List(TEXT("")), Alt_Spawn_List(TEXT("")), Condition_Type(TEXT("")) {}

	// DataTable Gray Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	int32 Start_Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	int32 End_Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	int32 Spawn_Interval;

	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	int32 Siege_Cycle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	FString Normal_Spawn_List;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	FString Special_Spawn_List;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	FString Alt_Spawn_List;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minion|Wave")
	FString Condition_Type;
};