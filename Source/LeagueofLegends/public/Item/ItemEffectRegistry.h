// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemEffectRegistry.generated.h"

class UItemPassiveEffectBase;
/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API UItemEffectRegistry : public UDataAsset
{
	GENERATED_BODY()
	
public:
	TSubclassOf< UItemPassiveEffectBase> FindPassiveClass(const FName& EffectName) const;
	// TSubclassOf<UItemActiveEffectBase> FindActiveClass(const FName& EffectName) const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Passive Registry")
	TMap<FName, TSubclassOf<UItemPassiveEffectBase>> PassiveMap;
	
	// UPROPERTY(EditDefaultsOnly, Category = "Active Registry")
	// TMap<FName, TSubclassOf<UItemActiveEffectBase>> ActiveMap;
};
