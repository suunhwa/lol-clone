// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Type/StatModifierTypes.h"
#include "ItemDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FItemEffectData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FString EffectName;

	UPROPERTY(VisibleAnywhere)
	float Value01 = 0.f;

	UPROPERTY(VisibleAnywhere)
	float Value02 = 0.f;

	UPROPERTY(VisibleAnywhere)
	FString Description;
};

UCLASS(BlueprintType)
class LEAGUEOFLEGENDS_API UItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// ===== Base =====
	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 ItemID = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	FString NameKR;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	FString NameEN;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 Price = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 RefundPrice = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	bool bIsActive = false;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	bool bIsConsumable = false;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	float Cooldown = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	int32 MaxStack = 0;

	UPROPERTY(VisibleAnywhere, Category = "Base")
	TSoftObjectPtr<UTexture2D> Icon;

	// ===== Stats =====
	// StatModifierComp.AddModifier()에 바로 넘길 수 있는 형태
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	TArray<FStatModifier> Stats;

	// ===== Effects =====
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	TArray<FItemEffectData> Effects;
};