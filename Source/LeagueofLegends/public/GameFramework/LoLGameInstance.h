// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Data/ChampionData.h"
#include "Engine/GameInstance.h"
#include "Type/RiftTypes.h"
#include "LoLGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class LEAGUEOFLEGENDS_API ULoLGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadWrite)
	FString Nickname;
	
	UPROPERTY(BlueprintReadWrite)
	EMatchMode SelectedMode = EMatchMode::SummonersRift;
	
	UPROPERTY(BlueprintReadWrite)
	UChampionData* SelectedChampion;
};
