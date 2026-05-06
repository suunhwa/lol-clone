// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ItemSettings.generated.h"

class UItemEffectRegistry;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="LoL Item System Settings"))
class LEAGUEOFLEGENDS_API UItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Registry")
	TSoftObjectPtr<UItemEffectRegistry> ItemEffectRegistry;
};
