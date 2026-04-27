// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/RiftTypes.h"
#include "Targetable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UTargetable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LEAGUEOFLEGENDS_API ITargetable
{
	GENERATED_BODY()

public:
	virtual bool IsTargetable() const = 0;
	virtual FVector GetTargetLocation() const = 0;
	virtual ETeam GetTeam() const = 0;
};
