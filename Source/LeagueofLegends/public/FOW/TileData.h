// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TileData.generated.h"

UENUM()
enum class ETileType : uint8
{
	None UMETA(DisplayName = "None"),
	Floor UMETA(DisplayName = "Floor"),
	Wall UMETA(DisplayName = "Wall"),
};

USTRUCT(BlueprintType)
struct FTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETileType Type = ETileType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsVisible = false;

	bool IsInMap() const
	{
		return Type != ETileType::None;
	}
};
