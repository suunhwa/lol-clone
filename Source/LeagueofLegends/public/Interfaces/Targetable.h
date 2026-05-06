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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targetable")
	bool IsTargetable() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targetable")
	FVector GetTargetLocation() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targetable")
	ETeam GetTeam() const;
};
