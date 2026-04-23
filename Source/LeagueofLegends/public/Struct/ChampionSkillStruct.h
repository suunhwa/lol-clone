// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChampionSkillStruct.generated.h"


USTRUCT(BlueprintType)
struct FMasterSkillCoreRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FMasterSkillCoreRow()
        : Skill_ID(TEXT("")), Name_KR(TEXT("")), Name_EN(TEXT(""))
        , Skill_Key(TEXT("")), Damage_Type(TEXT("")), Effect_Tag(TEXT("")) {}

    // Data Yellow Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Skill_ID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Name_KR;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Name_EN;

    // Data Blue Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Skill_Key; // P, Q, W, E, R 등

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    TArray<FString> Cost_Type; // Mana, HP, Energy 등

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Damage_Type; // AD, AP, None 등
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Core")
    FString Effect_Tag; // Mechanics 테이블 참조용 외래키
};

USTRUCT(BlueprintType)
struct FDetailSkillStatsRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FDetailSkillStatsRow()
        : Skill_ID(TEXT("")), Step(0), BaseValue(0.0f), Cost(0.0f), CoolDown(0.0f) {}

    // Data Yellow Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    FString Skill_ID;

    // Data Blue Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    int32 Step; // 스킬 레벨 (1~5)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    float BaseValue; // 기본 깡딜/수치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    TArray<FString> FactorStat; // 참조할 스탯 종류 (배열)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    TArray<float> Coefficient; // 각 스탯 계수 가중치 (배열)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    float Cost; // 소모량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Detail")
    float CoolDown; // 재사용 대기시간
};

USTRUCT(BlueprintType)
struct FSkillMechanicsRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    FSkillMechanicsRow()
        : Param1_Name(TEXT("")), Param1_Value(0.0f)
        , Param2_Name(TEXT("")), Param2_Value(0.0f), Logic_Ref(TEXT("")) {}

    // Data Blue Section
    // 첫 번째 매커니즘 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Mechanics")
    FString Param1_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Mechanics")
    float Param1_Value;

    // 두 번째 매커니즘 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Mechanics")
    FString Param2_Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Mechanics")
    float Param2_Value;
    
    // Data Red Section
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Mechanics")
    FString Logic_Ref;
};