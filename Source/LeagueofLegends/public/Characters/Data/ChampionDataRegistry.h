// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ChampionDataRegistry.generated.h"

class UChampionData;

// 챔피언 DataAsset 전체 목록을 관리하는 레지스트리.
// 새 챔피언 추가 시 이 에셋의 배열에만 넣으면 됨.
// 에셋 경로: /Game/Data/DA_ChampionRegistry
UCLASS()
class LEAGUEOFLEGENDS_API UChampionDataRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Champions")
	TArray<TObjectPtr<UChampionData>> AllChampions;
};
