// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SightProvider.generated.h"

UENUM(BlueprintType)
enum class ERiftTeam : uint8
{
	None, Red, Blue
};

// This class does not need to be modified.
UINTERFACE()
class USightProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEAGUEOFLEGENDS_API ISightProvider
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual FVector GetSightOrigin() const = 0;
	virtual float GetSightRange() const = 0;
	virtual bool IsStatic() const = 0;
	virtual ERiftTeam GetTeam() const = 0;
};
