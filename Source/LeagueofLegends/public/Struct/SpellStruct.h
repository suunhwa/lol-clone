#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SpellStruct.generated.h"


USTRUCT(BlueprintType)
struct FSpellBaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSpellBaseRow() 
		: Name_KR(TEXT("")), Name_EN(TEXT("")), Cooldown(0), Range(0)
		, BaseValue(0), ValueGrowth(0), MaxLevel(0), TargetLogic_ID(TEXT("")), EffectTag(TEXT("")) {}

	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString Name_KR;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString Name_EN;
	
	// DataTable Purple Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float Range;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float BaseValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float ValueGrowth;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	int32 MaxLevel;
	
	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString TargetLogic_ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString EffectTag;
};

USTRUCT(BlueprintType)
struct FSpellTargetingRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSpellTargetingRow() 
		: TargetStrategy(TEXT("")), MaxTarget(0), AffectsAlly(false)
		, PriorityType(TEXT("")), SearchRadius(0) {}

	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString TargetStrategy;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	int32 MaxTarget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	bool AffectsAlly;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString PriorityType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	int32 SearchRadius;
	
};

USTRUCT(BlueprintType)
struct FSpellSecondaryEffectRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FSpellSecondaryEffectRow() 
		: SecondaryValue(0), Duration(0), TickInterval(0), Stackable(false), Description(TEXT("")) {}

	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float SecondaryValue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float Duration;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	float TickInterval;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	bool Stackable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spell|Base")
	FString Description;
	
};
