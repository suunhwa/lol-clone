
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ObjectStruct.generated.h"


USTRUCT(BlueprintType)
struct FObjectBaseRow : public FTableRowBase
{
	GENERATED_BODY()
	
public :
	FObjectBaseRow() 
		:  Object_ID(0), Object_Type(TEXT(""))
		, Base_HP(0), HP_Regen(0)
		, Base_AD(0.0f), AD_Growth_Per_Min(0.0f), Max_AD(0.0f)
		, Atk_Range(0.0f), Atk_Speed(0.0f)
		, Required_Target_ID(TEXT("")), Description(TEXT("")) {}
	
	// Data Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	int32 Object_ID;
	
	// Data Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	FString Object_Type;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float Base_HP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float HP_Regen;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float Base_AD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float AD_Growth_Per_Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float Max_AD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	float Atk_Range;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	
	float Atk_Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	FString Required_Target_ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Base")
	FString Description;
};

USTRUCT(BlueprintType)
struct FObjectRewardRow : public FTableRowBase
{
	GENERATED_BODY()
	
public :
	FObjectRewardRow() 
		:  Object_ID(0), Plate_Gold(0)
		, Total_Plates(0), Global_Gold(0)
		, Last_Hit_Gold(0), Global_Exp(0)
		, bIncludeDead(false), bGlobalDist(false)
		, Exp_Range(0), Description(TEXT("")) {}
	
	// Data Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	int32 Object_ID;
	
	// Data Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Plate_Gold;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Total_Plates;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Global_Gold;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Last_Hit_Gold;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Global_Exp;
	
	// Data Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	bool bIncludeDead;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	bool bGlobalDist;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	float Exp_Range;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Reward")
	FString Description;
	
};

USTRUCT(BlueprintType)
struct FObjectMechanicsRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FObjectMechanicsRow()
		: Object_ID(0)
		, Heating_Rate(0.0f)
		, Max_Heating(0)
		, Plate_Expiry_Time(0.0f)
		, Base_Armor_After_Expiry(0.0f)
		, Plate_Armor_Bonus(0.0f)
		, Spawn_Unit_ID(0)
		, Respawn_Time(0.0f)
	{}

	// Data Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	int32 Object_ID;

	// Data Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	float Heating_Rate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	int32 Max_Heating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	float Plate_Expiry_Time;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	float Base_Armor_After_Expiry;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	float Plate_Armor_Bonus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	int32 Spawn_Unit_ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object|Mechanics")
	float Respawn_Time;
};
