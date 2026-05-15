// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StatModifierTypes.generated.h"

UENUM(BlueprintType)
enum class ELolStatType  : uint8
{
	// Survival
	HP,
	HPRegen,
	HPRegenPercent,
	
	// Resouce
	MP,
	MPRegen,
	MPRegenPercent,
	
	// Physical
	AD,
	ASRatio,
	Crit,
	ADPenFlat,
	ADPenRatio,
	
	// Magic
	AP,
	AH,
	APPenFlat,
	APPenRatio,
	
	// Defense
	Armor,
	MR,
	Tenacity,
	
	// Utility
	MS,
	MSPercent,
	LS,
	Omnivamp,
	HealShieldPower,
	
	FinalDamagePercent,
	
	// Economic
	Gold,
};

UENUM(BlueprintType)
enum class EModifierOp : uint8
{
	Add,  // 덧셈 — 아이템 스탯, 버프 수치
	Mul,  // 곱셈 — 퍼센트 증가
};

USTRUCT(BlueprintType)
struct FStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELolStatType StatType = ELolStatType::AD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EModifierOp Op = EModifierOp::Add;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.f;
};

USTRUCT(BlueprintType)
struct FStatModifierHandle
{
	GENERATED_BODY()

	UPROPERTY()
	int32 ID = -1;

	bool IsValid() const { return ID != -1; }
	bool operator==(const FStatModifierHandle& Other) const { return ID == Other.ID; }
};


