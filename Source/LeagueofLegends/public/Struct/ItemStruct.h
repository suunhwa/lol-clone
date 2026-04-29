#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemStruct.generated.h"

USTRUCT(BlueprintType)
struct FItemBaseRow : public FTableRowBase
{
	GENERATED_BODY()

	FItemBaseRow() : Item_ID(0), Effect_ID_R(TEXT("")), Price(0), RefundPrice(0), Recipe(TEXT("")), 
	Is_Active(false), Is_Consumable(false), Cooldown(0), Max_Stack(0), Name_KR(TEXT("")), Name_EN(TEXT("")) {}

	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	int32 Item_ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	FString Effect_ID_R;
	
	// DataTable Blue Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	int32 RefundPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	FString Recipe;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	bool Is_Active;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	bool Is_Consumable;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	float Cooldown;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	int32 Max_Stack;
	
	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	FString Name_KR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Base")
	FString Name_EN;
};

USTRUCT(BlueprintType)
struct FItemStatRow : public FTableRowBase
{
	GENERATED_BODY()

	FItemStatRow() : Item_ID(0), Stat_Type_R(TEXT("")), Stat_Value(0.f), Description(TEXT("")) {}

	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stat")
	int32 Item_ID;

	// DataTable Red Section 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stat")
	FString Stat_Type_R;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stat")
	float Stat_Value;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Stat")
	FString Description;
};

USTRUCT(BlueprintType)
struct FItemEffectRow : public FTableRowBase
{
	GENERATED_BODY()
	
	FItemEffectRow() : Effect_Name(TEXT("")), Stat_Type_R(TEXT("")), Value01(0.f), Value02(0.f), Description(TEXT("")) {}
	
	// DataTable Yellow Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	FString Effect_Name;
	
	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	FString Stat_Type_R;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	float Value01;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	float Value02;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effect")
	FString Description;
};

USTRUCT(BlueprintType)
struct FItemStatTypeRow : public FTableRowBase
{
	GENERATED_BODY()

	FItemStatTypeRow() : Category(TEXT("")), StatType(TEXT("")), Op(TEXT("")) {}

	// DataTable Red Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StatType")
	FString Category;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StatType")
	FString StatType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|StatType")
	FString Op;
};