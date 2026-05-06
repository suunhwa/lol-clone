// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SightProvider.generated.h"

UENUM(BlueprintType)
enum class ERiftSightTag : uint8
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
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sight")
	FVector GetSightOrigin() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sight")
	float GetSightRange() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sight")
	bool IsStatic() const;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sight")
	ERiftSightTag GetSightTag() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sight")
	bool IsHideable() const;
};
