// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChampionStatStruct.generated.h"


USTRUCT(BlueprintType)
struct FChampionBaseRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FChampionBaseRow()
        : Name_KR(TEXT("")), Name_EN(TEXT("")), ResourceType(TEXT(""))
        , Windup(0.0f), MissileSpeed(0.0f), Radius(0.0f), MoveSpeed(0.0f)
        , AttackRange(0.0f), StatValues_ID(0), Growth_ID(0), Description(TEXT("")) {}

    // Data Yellow Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    FString Name_KR;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    FString Name_EN;

    // Data Blue Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    FString ResourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    float Windup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    float MissileSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    float AttackRange;

    // Data Red Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    int32 StatValues_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    int32 Growth_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Base")
    FString Description;
};


USTRUCT(BlueprintType)
struct FChampionStatRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FChampionStatRow()
        : HP(0), MP(0), AD(0), Armor(0)
        , MR(0), AS_Ratio(0), AP(0), Crit_Mult(0)
        , AS_Base(0), Regen_HP(0), Description(TEXT("")) {}

    // Data Blue Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float HP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float MP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float AD;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float Armor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float MR;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float AS_Ratio;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float AP;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float Crit_Mult;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float AS_Base;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    float Regen_HP;
    
    // Data Red Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Stat")
    FString Description;
    
};

USTRUCT(BlueprintType)
struct FChampionGrowthRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FChampionGrowthRow()
        : HP_G(0), MP_G(0), AD_G(0), Armor_G(0)
        , MR_G(0), AS_G_Pct(0), AP_G(0), MS_G(0)
        , Regen_HP_G(0), Description(TEXT("")) {}

    // Data Blue Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float HP_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float MP_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float AD_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float Armor_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float MR_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float AS_G_Pct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float AP_G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float MS_G;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    float  Regen_HP_G;
    
    // Data Red Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Champion|Growth")
    FString Description;
    
    
};